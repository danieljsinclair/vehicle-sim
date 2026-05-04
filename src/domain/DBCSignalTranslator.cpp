#include "vehicle-sim/domain/DBCSignalTranslator.h"
#include "vehicle-sim/domain/VehicleSignal.h"

#include <chrono>
#include <algorithm>

namespace vehicle_sim::domain {

namespace {
    constexpr std::size_t CAN_DATA_OFFSET = 2;
    constexpr std::size_t CAN_FRAME_SIZE = 10;
}

DBCSignalTranslator::DBCSignalTranslator(
    const VehicleConfig& config,
    const DBCParseResult& parseResult
) noexcept
    : factory_(config, parseResult)
    , parseResult_(parseResult)
{
}

std::optional<VehicleSignal> DBCSignalTranslator::translate(
    const std::vector<std::uint8_t>& rawData
) const noexcept {
    if (!isValidPacket(rawData)) {
        return std::nullopt;
    }

    const std::uint16_t canId = extractCANId(rawData);

    // Extract 8-byte data payload (bytes 2-9)
    std::vector<std::uint8_t> data(
        rawData.begin() + CAN_DATA_OFFSET,
        rawData.begin() + CAN_FRAME_SIZE
    );

    // Store in accumulated frames (thread-safe)
    {
        std::lock_guard<std::mutex> lock(frames_mutex_);
        accumulatedFrames_[canId] = std::move(data);
    }

    // Build signal from all accumulated frames
    const auto now = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );

    // We need to copy the accumulated frames to pass to build()
    // since we can't hold the lock while calling build()
    std::unordered_map<std::uint16_t, std::vector<std::uint8_t>> framesCopy;
    {
        std::lock_guard<std::mutex> lock(frames_mutex_);
        framesCopy = accumulatedFrames_;
    }

    return factory_.build(framesCopy, now);
}

bool DBCSignalTranslator::isValidPacket(
    const std::vector<std::uint8_t>& rawData
) const noexcept {
    return rawData.size() >= CAN_FRAME_SIZE;
}

std::vector<std::uint16_t> DBCSignalTranslator::getSupportedCANIds() const noexcept {
    std::vector<std::uint16_t> ids;
    for (const auto& [canId, _] : parseResult_.signalsByCanId) {
        ids.push_back(canId);
    }
    return ids;
}

void DBCSignalTranslator::reset() noexcept {
    std::lock_guard<std::mutex> lock(frames_mutex_);
    accumulatedFrames_.clear();
}

std::uint16_t DBCSignalTranslator::extractCANId(
    const std::vector<std::uint8_t>& frame
) noexcept {
    if (frame.size() < 2) return 0;
    return static_cast<std::uint16_t>(frame[0]) |
           (static_cast<std::uint16_t>(frame[1]) << 8);
}

} // namespace vehicle_sim::domain

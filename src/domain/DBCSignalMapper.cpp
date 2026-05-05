#include "vehicle-sim/domain/DBCSignalMapper.h"
#include "vehicle-sim/domain/CANDecoder.h"

#include <algorithm>
#include <cstdint>

namespace vehicle_sim::domain {

std::optional<double> DBCSignalMapper::mapSignal(
    const std::vector<std::uint8_t>& frame,
    const DBCSignalDefinition& definition
) noexcept {
    const std::size_t lastBit = definition.startBit + definition.bitLength - 1;
    if (lastBit >= frame.size() * 8) {
        return std::nullopt;
    }

    const auto rawBits = extractRawBits(frame, definition);
    if (definition.isSigned) {
        const auto signedVal = CANDecoder::toSigned(rawBits, definition.bitLength);
        double physical = static_cast<double>(signedVal) * definition.scale + definition.offset;
        return std::clamp(physical, definition.min, definition.max);
    }
    double physical = static_cast<double>(rawBits) * definition.scale + definition.offset;
    return std::clamp(physical, definition.min, definition.max);
}

std::optional<double> DBCSignalMapper::mapSignal(
    const std::vector<std::uint8_t>& frame,
    std::uint16_t canId,
    const std::string& signalName,
    const std::unordered_map<std::uint16_t,
        std::vector<DBCSignalDefinition>>& definitions
) noexcept {
    auto it = definitions.find(canId);
    if (it == definitions.end()) return std::nullopt;

    for (const auto& def : it->second) {
        if (def.name == signalName) {
            return mapSignal(frame, def);
        }
    }
    return std::nullopt;
}

std::uint64_t DBCSignalMapper::extractRawBits(
    const std::vector<std::uint8_t>& frame,
    const DBCSignalDefinition& definition
) noexcept {
    if (definition.bitLength == 0 || definition.bitLength > 64) return 0;

    std::uint64_t result = 0;

    if (definition.byteOrder == DBCByteOrder::Intel) {
        // Intel (@1): bit 0 = LSB of byte 0, sequential across bytes
        for (std::size_t i = 0; i < definition.bitLength; ++i) {
            const std::size_t bitPos = definition.startBit + i;
            const std::size_t byteIdx = bitPos / 8;
            const std::size_t bitIdx = bitPos % 8;

            if (byteIdx < frame.size() && (frame[byteIdx] & (1ULL << bitIdx))) {
                result |= (1ULL << i);
            }
        }
    } else {
        // Motorola (@0): MSB at startBit, reversed bit numbering within bytes
        // DBC bit n → byte = n/8, bit_within_byte = 7 - (n%8)
        // Signal MSB at startBit, bits progress in increasing DBC position
        for (std::size_t i = 0; i < definition.bitLength; ++i) {
            const std::size_t dbcBit = definition.startBit + i;
            const std::size_t byteIdx = dbcBit / 8;
            const std::size_t bitInByte = 7 - (dbcBit % 8);
            const std::size_t resultBit = definition.bitLength - 1 - i;

            if (byteIdx < frame.size() && (frame[byteIdx] & (1ULL << bitInByte))) {
                result |= (1ULL << resultBit);
            }
        }
    }

    return result;
}

} // namespace vehicle_sim::domain

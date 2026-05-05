#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include <memory>
#include "vehicle-sim/domain/ISignalTranslator.h"
#include "vehicle-sim/domain/ITimeProvider.h"

namespace vehicle_sim::domain {

/**
 * Abstract base class for CAN frame translators using Template Method pattern.
 *
 * Extracts common functionality shared by vehicle-specific CAN translators:
 * - CAN ID extraction from frame header (little-endian)
 * - Frame validation (size check, known CAN ID check)
 * - State accumulation across multiple frames
 * - VehicleSignal construction with injected time provider
 *
 * Concrete translators provide vehicle-specific behavior via:
 * - isKnownCANId(): which CAN IDs are supported
 * - decodeFrame(): decode signals for a specific CAN ID
 *
 * Frame format expected:
 *   [canId_lo, canId_hi, data_byte_0, ..., data_byte_7]
 */
class CANTranslatorBase : public ISignalTranslator {
public:
    explicit CANTranslatorBase(std::shared_ptr<ITimeProvider> timeProvider = nullptr);
    ~CANTranslatorBase() override;

    CANTranslatorBase(const CANTranslatorBase&) = delete;
    CANTranslatorBase& operator=(const CANTranslatorBase&) = delete;

    [[nodiscard]] bool isValidPacket(
        const std::vector<uint8_t>& rawData
    ) const noexcept override;

    [[nodiscard]] std::optional<VehicleSignal> translate(
        const std::vector<uint8_t>& rawData
    ) const noexcept override;

protected:
    static constexpr std::size_t CAN_DATA_OFFSET = 2;
    static constexpr std::size_t CAN_FRAME_SIZE = 10;

    [[nodiscard]] static uint16_t extractCANId(
        const std::vector<uint8_t>& frame
    ) noexcept;

    // Pure virtual methods for concrete translators to implement
    [[nodiscard]] virtual bool isKnownCANId(uint16_t id) const noexcept = 0;
    virtual void decodeFrame(uint16_t canId, const std::vector<uint8_t>& data) const = 0;

    // Shared signal constants for CAN IDs shared across vehicles
    static constexpr uint16_t CAN_ID_DI_SYSTEM = 280;
    static constexpr uint16_t CAN_ID_SCCM_STEER = 297;

    // DI_systemStatus signal constants
    static constexpr std::size_t DI_PEDAL_START_BIT = 32;
    static constexpr std::size_t DI_PEDAL_BIT_LENGTH = 8;
    static constexpr double DI_PEDAL_SCALE = 0.4;

    static constexpr std::size_t DI_BRAKE_STATE_START_BIT = 17;
    static constexpr std::size_t DI_BRAKE_STATE_BIT_LENGTH = 2;
    static constexpr double BRAKE_PEDAL_PRESSED_PERCENT = 50.0;

    // SCCM_steeringAngleSensor signal constants
    static constexpr std::size_t SCCM_STEER_START_BIT = 16;
    static constexpr std::size_t SCCM_STEER_BIT_LENGTH = 14;
    static constexpr double SCCM_STEER_SCALE = 0.1;
    static constexpr double SCCM_STEER_OFFSET = -819.2;

    // Shared decode methods for common CAN IDs
    void decodeDISystem(const std::vector<uint8_t>& data) const;
    void decodeSCCMSteering(const std::vector<uint8_t>& data) const;

    [[nodiscard]] std::optional<VehicleSignal> buildSignal() const noexcept;

    std::shared_ptr<ITimeProvider> timeProvider_;

    mutable double lastSpeedKmh_ = 0.0;
    mutable double lastThrottlePercent_ = 0.0;
    mutable double lastAccelerationG_ = 0.0;
    mutable double lastBrakePercent_ = 0.0;
    mutable double lastSteeringAngleDeg_ = 0.0;
};

} // namespace vehicle_sim::domain

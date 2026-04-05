#pragma once

#include "vehicle-sim/domain/ISignalTranslator.h"

namespace vehicle_sim::domain {

/**
 * Tesla-specific BLE signal translator
 *
 * Concrete implementation of ISignalTranslator that parses Tesla Model Y
 * BLE OBD2 data packets and converts them to canonical VehicleSignal format.
 *
 * This class contains all Tesla-specific encoding logic. Tesla implementation
 * details are completely hidden behind this boundary - upper layers only see
 * standard OBD2 VehicleSignal objects.
 *
 * Packet format (mock for now - actual Tesla format TBD):
 * [0xAA][0x55][length][throttle][speed_lo][speed_hi][accel][brake][checksum]
 * - throttle: uint8_t (0-100 percentage)
 * - speed: uint16_t little endian (km/h)
 * - accel: int8_t (acceleration G * 10, signed)
 * - brake: uint8_t (0-100 percentage)
 * - checksum: uint8_t (sum of all bytes modulo 256)
 */
class TeslaSignalTranslator final : public ISignalTranslator {
public:
    TeslaSignalTranslator() = default;
    ~TeslaSignalTranslator() override = default;

    // No copy/move needed for this simple translator
    TeslaSignalTranslator(const TeslaSignalTranslator&) = delete;
    TeslaSignalTranslator& operator=(const TeslaSignalTranslator&) = delete;

    [[nodiscard]] std::optional<VehicleSignal> translate(
        const std::vector<std::uint8_t>& rawData
    ) const noexcept override;

    [[nodiscard]] bool isValidPacket(
        const std::vector<std::uint8_t>& rawData
    ) const noexcept override;

private:
    static constexpr std::uint8_t HEADER_0 = 0xAA;
    static constexpr std::uint8_t HEADER_1 = 0x55;
    static constexpr std::size_t MIN_PACKET_SIZE = 9;
    static constexpr std::size_t PAYLOAD_START = 3;
    static constexpr std::size_t THROTTLE_OFFSET = 0;
    static constexpr std::size_t SPEED_LO_OFFSET = 1;
    static constexpr std::size_t SPEED_HI_OFFSET = 2;
    static constexpr std::size_t ACCEL_OFFSET = 3;
    static constexpr std::size_t BRAKE_OFFSET = 4;

    [[nodiscard]] std::uint8_t calculateChecksum(
        const std::vector<std::uint8_t>& packet
    ) const noexcept;
};

} // namespace vehicle_sim::domain

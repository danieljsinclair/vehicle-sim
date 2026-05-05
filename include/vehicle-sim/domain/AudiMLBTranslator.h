#pragma once

#include <vector>
#include <cstdint>
#include "vehicle-sim/domain/CANTranslatorBase.h"

namespace vehicle_sim::domain {

/**
 * CAN frame decoder for Audi e-tron 2021 (MLB Evo platform).
 *
 * Decodes raw CAN frames using verified signal definitions from
 * commaai/opendbc vw_mlb.dbc. Each CAN ID maps to a specific
 * message containing one or more signals.
 *
 * Frame format expected by translate():
 *   [canId_lo, canId_hi, data_byte_0, ..., data_byte_7]
 *
 * Supported CAN IDs:
 *   256 (0x100) ESP_01   — vehicle speed
 *   257 (0x101) ESP_02   — longitudinal/lateral acceleration
 *   259 (0x103) ESP_03   — wheel speeds (4 corners)
 *   262 (0x106) ESP_05   — brake pressure
 *   280 (0x118) DI_systemStatus — accelerator pedal, brake state
 *   297 (0x129) SCCM_steeringAngleSensor — steering angle
 */
class AudiMLBTranslator final : public CANTranslatorBase {
public:
    AudiMLBTranslator();
    ~AudiMLBTranslator() override = default;

    AudiMLBTranslator(const AudiMLBTranslator&) = delete;
    AudiMLBTranslator& operator=(const AudiMLBTranslator&) = delete;

private:
    // CAN IDs from vw_mlb.dbc (Audi-specific)
    static constexpr uint16_t CAN_ID_ESP_01 = 256;
    static constexpr uint16_t CAN_ID_ESP_02 = 257;
    static constexpr uint16_t CAN_ID_ESP_03 = 259;
    static constexpr uint16_t CAN_ID_ESP_05 = 262;

    // ESP_01 signal: ESP_v_Signal (vehicle speed)
    static constexpr std::size_t ESP01_SPEED_START_BIT = 32;
    static constexpr std::size_t ESP01_SPEED_BIT_LENGTH = 16;
    static constexpr double ESP01_SPEED_SCALE = 0.01;        // km/h per count

    // ESP_02 signal: ESP_Laengsbeschl (longitudinal acceleration)
    static constexpr std::size_t ESP02_ACCEL_START_BIT = 24;
    static constexpr std::size_t ESP02_ACCEL_BIT_LENGTH = 10;
    static constexpr double ESP02_ACCEL_SCALE = 0.03125;     // m/s^2 per count
    static constexpr double ESP02_ACCEL_OFFSET = -16.0;      // m/s^2
    static constexpr double MS2_TO_G = 9.81;                 // m/s^2 per g

    // ESP_05 signal: ESP_Bremsdruck (brake pressure)
    static constexpr std::size_t ESP05_BRAKE_START_BIT = 16;
    static constexpr std::size_t ESP05_BRAKE_BIT_LENGTH = 10;
    static constexpr double ESP05_BRAKE_SCALE = 0.3;         // bar per count
    static constexpr double ESP05_BRAKE_OFFSET = -30.0;      // bar
    static constexpr double BRAKE_PRESSURE_MAX_BAR = 200.0;  // 0 bar = 0%, 200 bar = 100%

    [[nodiscard]] bool isKnownCANId(uint16_t id) const noexcept override;
    void decodeFrame(uint16_t canId, const std::vector<uint8_t>& data) const override;

    void decodeESP01(const std::vector<uint8_t>& data) const;
    void decodeESP02(const std::vector<uint8_t>& data) const;
    void decodeESP03(const std::vector<uint8_t>& data) const;
    void decodeESP05(const std::vector<uint8_t>& data) const;
};

} // namespace vehicle_sim::domain

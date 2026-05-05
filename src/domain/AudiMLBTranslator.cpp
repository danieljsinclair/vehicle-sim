#include "vehicle-sim/domain/AudiMLBTranslator.h"
#include "vehicle-sim/domain/CANDecoder.h"

namespace vehicle_sim::domain {

AudiMLBTranslator::AudiMLBTranslator() = default;

bool AudiMLBTranslator::isKnownCANId(uint16_t id) const noexcept {
    return id == CAN_ID_ESP_01 || id == CAN_ID_ESP_02 ||
           id == CAN_ID_ESP_03 || id == CAN_ID_ESP_05 ||
           id == CAN_ID_DI_SYSTEM || id == CAN_ID_SCCM_STEER;
}

void AudiMLBTranslator::decodeFrame(uint16_t canId, const std::vector<uint8_t>& data) const {
    switch (canId) {
        case CAN_ID_ESP_01:   decodeESP01(data);   break;
        case CAN_ID_ESP_02:   decodeESP02(data);   break;
        case CAN_ID_ESP_03:   decodeESP03(data);   break;
        case CAN_ID_ESP_05:   decodeESP05(data);   break;
        case CAN_ID_DI_SYSTEM: decodeDISystem(data); break;
        case CAN_ID_SCCM_STEER: decodeSCCMSteering(data); break;
    }
}

// CAN 256 (0x100) ESP_01
// ESP_v_Signal: start bit 32, 16-bit unsigned, scale 0.01, unit km/h
void AudiMLBTranslator::decodeESP01(const std::vector<uint8_t>& data) const {
    auto speed = CANDecoder::extractSignal(data, ESP01_SPEED_START_BIT, ESP01_SPEED_BIT_LENGTH, ESP01_SPEED_SCALE, 0.0, false);
    if (speed.has_value()) {
        lastSpeedKmh_ = *speed;
    }
}

// CAN 257 (0x101) ESP_02
// ESP_Laengsbeschl: start bit 24, 10-bit unsigned, scale 0.03125, offset -16, unit m/s^2
// ESP_Querbeschleunigung: start bit 48, 8-bit unsigned, scale 0.01, offset -1.27, unit g
void AudiMLBTranslator::decodeESP02(const std::vector<uint8_t>& data) const {
    auto longitudinal = CANDecoder::extractSignal(data, ESP02_ACCEL_START_BIT, ESP02_ACCEL_BIT_LENGTH, ESP02_ACCEL_SCALE, ESP02_ACCEL_OFFSET, false);
    if (longitudinal.has_value()) {
        lastAccelerationG_ = *longitudinal / MS2_TO_G;
    }
}

// CAN 259 (0x103) ESP_03 — wheel speeds
// Not directly mapped to VehicleSignal fields in current schema
void AudiMLBTranslator::decodeESP03(const std::vector<uint8_t>& data) const {
    // ESP_RadHL_Hz: start bit 0, 12-bit, scale 0.1, unit km/h
    // ESP_RadHR_Hz: start bit 16, 12-bit, scale 0.1, unit km/h
    // ESP_RadVL_Hz: start bit 32, 12-bit, scale 0.1, unit km/h
    // ESP_RadVR_Hz: start bit 48, 12-bit, scale 0.1, unit km/h
    // Could compute differential for slip detection in future
    (void)data;
}

// CAN 262 (0x106) ESP_05
// ESP_Bremsdruck: start bit 16, 10-bit unsigned, scale 0.3, offset -30, unit bar
void AudiMLBTranslator::decodeESP05(const std::vector<uint8_t>& data) const {
    auto brakeBar = CANDecoder::extractSignal(data, ESP05_BRAKE_START_BIT, ESP05_BRAKE_BIT_LENGTH, ESP05_BRAKE_SCALE, ESP05_BRAKE_OFFSET, false);
    if (brakeBar.has_value()) {
        if (*brakeBar <= 0.0) {
            lastBrakePercent_ = 0.0;
        } else {
            lastBrakePercent_ = std::min(*brakeBar / BRAKE_PRESSURE_MAX_BAR, 1.0) * 100.0;
        }
    }
}

} // namespace vehicle_sim::domain

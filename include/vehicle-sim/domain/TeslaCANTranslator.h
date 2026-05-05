#pragma once

#include <vector>
#include <cstdint>
#include "vehicle-sim/domain/CANTranslatorBase.h"

namespace vehicle_sim::domain {

/**
 * CAN frame decoder for Tesla Model 3/Y.
 *
 * Decodes raw CAN frames using verified signal definitions from
 * joshwardell/model3dbc.
 *
 * Frame format expected by translate():
 *   [canId_lo, canId_hi, data_byte_0, ..., data_byte_7]
 *
 * Supported CAN IDs (verified from model3dbc):
 *   280 (0x118) DI_systemStatus — accelerator, brake, gear
 *   297 (0x129) SCCM_steeringAngleSensor — steering angle
 *
 * NOTE: Additional Tesla-specific CAN IDs (e.g., UI_gpsVehicleSpeed CAN 985,
 * RCM_inertial2 CAN 273) will be added once exact DBC signal bit positions
 * are verified against the model3dbc file.
 */
class TeslaCANTranslator final : public CANTranslatorBase {
public:
    TeslaCANTranslator();
    ~TeslaCANTranslator() override = default;

    TeslaCANTranslator(const TeslaCANTranslator&) = delete;
    TeslaCANTranslator& operator=(const TeslaCANTranslator&) = delete;

private:
    [[nodiscard]] bool isKnownCANId(uint16_t id) const noexcept override;
    void decodeFrame(uint16_t canId, const std::vector<uint8_t>& data) const override;
};

} // namespace vehicle_sim::domain

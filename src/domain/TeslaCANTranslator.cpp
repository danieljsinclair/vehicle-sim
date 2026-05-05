#include "vehicle-sim/domain/TeslaCANTranslator.h"

namespace vehicle_sim::domain {

TeslaCANTranslator::TeslaCANTranslator() = default;

bool TeslaCANTranslator::isKnownCANId(uint16_t id) const noexcept {
    return id == CAN_ID_DI_SYSTEM || id == CAN_ID_SCCM_STEER;
}

void TeslaCANTranslator::decodeFrame(uint16_t canId, const std::vector<uint8_t>& data) const {
    switch (canId) {
        case CAN_ID_DI_SYSTEM: decodeDISystem(data); break;
        case CAN_ID_SCCM_STEER: decodeSCCMSteering(data); break;
    }
}

} // namespace vehicle_sim::domain

#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include "vehicle-sim/domain/VehicleSignal.h"

namespace vehicle_sim::domain {

/**
 * Pure virtual interface for translating raw BLE data to canonical VehicleSignal
 *
 * This is the boundary layer adapter that hides Tesla-specific implementation details.
 * All upper layers only see standard OBD2 format VehicleSignal objects.
 * Concrete implementations (TeslaTranslator, MockTranslator) must implement this interface.
 *
 * The interface is noexcept and testable with mocks for dependency injection.
 */
class ISignalTranslator {
public:
    virtual ~ISignalTranslator() = default;

    /**
     * Translate raw BLE packet data to canonical VehicleSignal
     *
     * @param rawData Raw bytes from BLE device
     * @return VehicleSignal if translation successful, std::nullopt otherwise
     */
    [[nodiscard]] virtual std::optional<VehicleSignal> translate(
        const std::vector<std::uint8_t>& rawData
    ) const noexcept = 0;

    /**
     * Validate whether a raw packet can be translated
     *
     * @param rawData Raw bytes from BLE device
     * @return true if packet is valid and translatable, false otherwise
     */
    [[nodiscard]] virtual bool isValidPacket(
        const std::vector<std::uint8_t>& rawData
    ) const noexcept = 0;
};

} // namespace vehicle_sim::domain

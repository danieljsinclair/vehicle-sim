#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace vehicle_sim::domain {

enum class VehicleMake {
    Unknown,
    Tesla,
    Audi,
    Volkswagen,
    BMW,
    MercedesBenz,
    Generic
};

struct VehicleDetectionResult {
    VehicleMake make;
    std::string vin;
    std::string wmi;
    std::string suggestedVehicleId;
    bool isElectric;
};

class VehicleDetector {
public:
    VehicleDetector() = default;

    [[nodiscard]] static std::vector<std::uint8_t> buildVINQuery();
    [[nodiscard]] static std::vector<std::uint8_t> buildFuelTypeQuery();

    bool feedVINResponse(const std::vector<std::uint8_t>& response);
    bool feedFuelTypeResponse(const std::vector<std::uint8_t>& response);

    [[nodiscard]] std::optional<VehicleDetectionResult> getResult() const noexcept;

    [[nodiscard]] static VehicleMake decodeWMI(const std::string& wmi);
    [[nodiscard]] static std::string makeToConfigId(VehicleMake make, bool isElectric);
    [[nodiscard]] static std::string extractVINFromResponse(const std::vector<std::uint8_t>& response);

    void reset();

private:
    std::string vin_;
    std::optional<bool> isElectric_;
    std::optional<VehicleDetectionResult> result_;
    void completeDetection();
};

} // namespace vehicle_sim::domain

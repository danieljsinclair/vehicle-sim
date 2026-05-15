#include "vehicle-sim/presentation/VehicleSignalFormatter.h"
#include "vehicle-sim/domain/VehicleSignal.h"
#include "vehicle-sim/domain/VehicleConfig.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace vehicle_sim::presentation {

std::string formatTelemetryRow(const domain::VehicleSignal& signal, int count) {
    std::ostringstream out;
    printTelemetryRow(out, signal, count);
    return out.str();
}

std::string formatTelemetryHeader(const domain::VehicleConfig& config) {
    std::ostringstream out;
    printTelemetryHeader(out, config);
    return out.str();
}

void printTelemetryRow(std::ostream& out, const domain::VehicleSignal& signal, int count) {
    out << "[" << count << "] ";
    out << "Throttle: " << std::setw(5) << std::fixed << std::setprecision(1)
        << signal.getThrottlePercent() << "%  ";
    out << "Speed: " << std::setw(5) << std::fixed << std::setprecision(1)
        << signal.getSpeedKmh() << " km/h  ";
    out << "Brake: " << std::setw(5) << std::fixed << std::setprecision(1)
        << signal.getBrakePercent() << "%  ";
    out << "Accel: " << std::setw(5) << std::fixed << std::setprecision(2)
        << signal.getAccelerationG() << " G  ";
    out << "Steer: " << std::setw(6) << std::fixed << std::setprecision(1)
        << signal.getSteeringAngleDeg() << "°  ";
    out << "Motor: " << std::setw(5) << std::fixed << std::setprecision(0)
        << signal.getMotorRpm() << " rpm  ";
    out << "HV: " << std::setw(5) << std::fixed << std::setprecision(1)
        << signal.getMotorHvVoltage() << "V  ";
    out << "Curr: " << std::setw(5) << std::fixed << std::setprecision(1)
        << signal.getMotorHvCurrent() << "A  ";
    out << "Trq: " << std::setw(6) << std::fixed << std::setprecision(1)
        << signal.getMotorTorqueNm() << " Nm  ";
    out << "Gear: " << std::setw(1) << signal.getGearSelector() << "\n";
}

void printTelemetryHeader(std::ostream& out, const domain::VehicleConfig& config) {
    out << "\n" << std::string(TERMINAL_SEPARATOR_WIDTH, '=') << "\n";
    out << config.vehicleName << " Real-Time Telemetry\n";
    out << std::string(TERMINAL_SEPARATOR_WIDTH, '=') << "\n\n";
}

} // namespace vehicle_sim::presentation

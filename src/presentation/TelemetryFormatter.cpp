#include "vehicle-sim/TelemetryFormatter.h"
#include <sstream>
#include <iomanip>

namespace vehicle_sim {

TelemetryFormatter::TelemetryFormatter(Format format)
    : format_(format)
    , include_headers_(true)
    , delimiter_(',') {
}

void TelemetryFormatter::setFormat(Format format) {
    format_ = format;
}

void TelemetryFormatter::setIncludeHeaders(bool include) {
    include_headers_ = include;
}

void TelemetryFormatter::setDelimiter(char delimiter) {
    delimiter_ = delimiter;
}

std::string TelemetryFormatter::format(const PhysicsData& data) {
    std::ostringstream oss;

    switch (format_) {
        case Format::JSON: {
            oss << "{";
            oss << "\"timestamp\":" << data.timestamp << ",";
            oss << "\"rpm\":" << data.rpm << ",";
            oss << "\"speed\":" << data.speed << ",";
            oss << "\"throttle\":" << data.throttle_position << ",";
            oss << "\"torque\":" << data.torque << ",";
            oss << "\"acceleration\":" << data.acceleration << ",";
            oss << "\"gear\":" << data.gear << ",";
            oss << "\"brake\":" << (data.brake_active ? "true" : "false");
            oss << "}";
            break;
        }

        case Format::CSV: {
            if (include_headers_) {
                oss << "timestamp,rpm,speed,throttle,torque,acceleration,gear,brake" << std::endl;
            }
            oss << std::fixed << std::setprecision(6);
            oss << data.timestamp << delimiter_
                << data.rpm << delimiter_
                << data.speed << delimiter_
                << data.throttle_position << delimiter_
                << data.torque << delimiter_
                << data.acceleration << delimiter_
                << data.gear << delimiter_
                << (data.brake_active ? "1" : "0");
            break;
        }

        case Format::PLAINTEXT: {
            oss << "Telemetry:" << std::endl;
            oss << "  Time: " << data.timestamp << "s" << std::endl;
            oss << "  RPM: " << data.rpm << std::endl;
            oss << "  Speed: " << data.speed << " km/h" << std::endl;
            oss << "  Throttle: " << (data.throttle_position * 100.0) << "%" << std::endl;
            oss << "  Torque: " << data.torque << " Nm" << std::endl;
            oss << "  Acceleration: " << data.acceleration << " m/s²" << std::endl;
            oss << "  Gear: " << data.gear << std::endl;
            oss << "  Brake: " << (data.brake_active ? "ON" : "OFF");
            break;
        }
    }

    return oss.str();
}

} // namespace vehicle_sim

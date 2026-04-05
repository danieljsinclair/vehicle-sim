#pragma once

#include <string>
#include "PhysicsEngine.h"

namespace vehicle_sim {

enum class Format {
    JSON,
    CSV,
    PLAINTEXT
};

class TelemetryFormatter {
public:
    TelemetryFormatter(Format format = Format::JSON);

    // Format physics data according to configured format
    std::string format(const PhysicsData& data);

    // Set output format
    void setFormat(Format format);

    // Enable/disable headers (for CSV)
    void setIncludeHeaders(bool include);
    void setDelimiter(char delimiter);

private:
    Format format_;
    bool include_headers_;
    char delimiter_;
};

} // namespace vehicle_sim

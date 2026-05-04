#include <gtest/gtest.h>
#include <sstream>
#include "vehicle-sim/presentation/VehicleSignalFormatter.h"
#include "vehicle-sim/domain/VehicleSignal.h"
#include "vehicle-sim/domain/VehicleConfig.h"

using namespace vehicle_sim::presentation;
using namespace vehicle_sim::domain;

class VehicleSignalFormatterTest : public ::testing::Test {
protected:
    VehicleSignal makeSignal(double throttle, double speed, double accel, double brake) {
        return VehicleSignal(throttle, speed, accel, brake, 1000);
    }

    VehicleConfig makeConfig(const std::string& name) {
        return VehicleConfig("test.dbc", name, {}, "", false);
    }
};

TEST_F(VehicleSignalFormatterTest, FormatRowContainsAllFields) {
    auto signal = makeSignal(55.5, 120.3, 0.25, 10.0);
    std::string row = formatTelemetryRow(signal, 1);

    EXPECT_NE(row.find("Throttle"), std::string::npos);
    EXPECT_NE(row.find("Speed"), std::string::npos);
    EXPECT_NE(row.find("Brake"), std::string::npos);
    EXPECT_NE(row.find("Accel"), std::string::npos);
}

TEST_F(VehicleSignalFormatterTest, FormatRowStartsWithCount) {
    auto signal = makeSignal(0.0, 0.0, 0.0, 0.0);
    std::string row = formatTelemetryRow(signal, 42);

    EXPECT_NE(row.find("[42]"), std::string::npos);
}

TEST_F(VehicleSignalFormatterTest, FormatRowContainsKnownValues) {
    auto signal = makeSignal(55.5, 120.3, 0.25, 10.0);
    std::string row = formatTelemetryRow(signal, 1);

    EXPECT_NE(row.find("55.5"), std::string::npos);
    EXPECT_NE(row.find("120.3"), std::string::npos);
    EXPECT_NE(row.find("10.0"), std::string::npos);
    EXPECT_NE(row.find("0.25"), std::string::npos);
}

TEST_F(VehicleSignalFormatterTest, FormatRowWritesToStream) {
    auto signal = makeSignal(50.0, 100.0, 0.5, 5.0);
    std::ostringstream out;
    printTelemetryRow(out, signal, 3);

    std::string result = out.str();
    EXPECT_NE(result.find("[3]"), std::string::npos);
    EXPECT_NE(result.find("50.0"), std::string::npos);
}

TEST_F(VehicleSignalFormatterTest, FormatHeaderContainsVehicleName) {
    auto config = makeConfig("Test Vehicle X");
    std::string header = formatTelemetryHeader(config);

    EXPECT_NE(header.find("Test Vehicle X"), std::string::npos);
    EXPECT_NE(header.find("Real-Time Telemetry"), std::string::npos);
}

TEST_F(VehicleSignalFormatterTest, FormatHeaderHasSeparatorLines) {
    auto config = makeConfig("Test");
    std::string header = formatTelemetryHeader(config);

    std::string expected_sep(TERMINAL_SEPARATOR_WIDTH, '=');
    // Header has two separator lines
    EXPECT_NE(header.find(expected_sep), std::string::npos);
}

TEST_F(VehicleSignalFormatterTest, FormatHeaderWritesToStream) {
    auto config = makeConfig("Audi");
    std::ostringstream out;
    printTelemetryHeader(out, config);

    EXPECT_NE(out.str().find("Audi"), std::string::npos);
}

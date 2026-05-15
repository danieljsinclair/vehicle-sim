#include <gtest/gtest.h>
#include "vehicle-sim/domain/VehicleSignal.h"

using namespace vehicle_sim::domain;

// ================================================
// VehicleSignal Unit Tests
// TDD - Full test coverage for value object
// ================================================

TEST(VehicleSignalTest, ConstructsWithValidValues)
{
    const VehicleSignal signal(50.0, 100.0, 0.5, 25.0, 123456789ULL);

    EXPECT_DOUBLE_EQ(signal.getThrottlePercent(), 50.0);
    EXPECT_DOUBLE_EQ(signal.getSpeedKmh(), 100.0);
    EXPECT_DOUBLE_EQ(signal.getAccelerationG(), 0.5);
    EXPECT_DOUBLE_EQ(signal.getBrakePercent(), 25.0);
    EXPECT_EQ(signal.getTimestampUtcMs(), 123456789ULL);
}

TEST(VehicleSignalTest, ClampsThrottleToValidRange)
{
    EXPECT_DOUBLE_EQ(VehicleSignal(-10.0, 0, 0, 0, 0).getThrottlePercent(), 0.0);
    EXPECT_DOUBLE_EQ(VehicleSignal(150.0, 0, 0, 0, 0).getThrottlePercent(), 100.0);
}

TEST(VehicleSignalTest, ClampsSpeedToValidRange)
{
    EXPECT_DOUBLE_EQ(VehicleSignal(0, -5.0, 0, 0, 0).getSpeedKmh(), 0.0);
    EXPECT_DOUBLE_EQ(VehicleSignal(0, 400.0, 0, 0, 0).getSpeedKmh(), 300.0);
}

TEST(VehicleSignalTest, ClampsAccelerationToValidRange)
{
    EXPECT_DOUBLE_EQ(VehicleSignal(0, 0, -10.0, 0, 0).getAccelerationG(), -5.0);
    EXPECT_DOUBLE_EQ(VehicleSignal(0, 0, 10.0, 0, 0).getAccelerationG(), 5.0);
}

TEST(VehicleSignalTest, ClampsBrakeToValidRange)
{
    EXPECT_DOUBLE_EQ(VehicleSignal(0, 0, 0, -5.0, 0).getBrakePercent(), 0.0);
    EXPECT_DOUBLE_EQ(VehicleSignal(0, 0, 0, 120.0, 0).getBrakePercent(), 100.0);
}

TEST(VehicleSignalTest, ValueEqualityWorks)
{
    const VehicleSignal a(50.0, 100.0, 0.5, 25.0, 12345);
    const VehicleSignal b(50.0, 100.0, 0.5, 25.0, 12345);
    const VehicleSignal c(51.0, 100.0, 0.5, 25.0, 12345);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(VehicleSignalTest, ValueInequalityWorks)
{
    const VehicleSignal a(50.0, 100.0, 0.5, 25.0, 12345);
    const VehicleSignal b(51.0, 100.0, 0.5, 25.0, 12345);

    EXPECT_NE(a, b);
}

// NOTE: Immutability is a compile-time guarantee, not runtime behavior
// Immutability is enforced by compile-time through API design:
// - No mutator methods exist in VehicleSignal class
// - All methods are const
// - Return by value only
// This is a compile-time guarantee, not a runtime test

// ================================================
// VehicleSignal Extended Field Tests
// Tests for EV-specific fields (motorRpm, motorTorqueNm, gearSelector, etc.)
// ================================================

TEST(VehicleSignalTest, MotorRpmIsClampedToMax20000) {
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, 25000.0);
    EXPECT_DOUBLE_EQ(signal.getMotorRpm(), 20000.0);
}

TEST(VehicleSignalTest, MotorRpmIsClampedToMin0) {
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, -500.0);
    EXPECT_DOUBLE_EQ(signal.getMotorRpm(), 0.0);
}

TEST(VehicleSignalTest, MotorRpmAcceptsValidRange) {
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, 15000.5);
    EXPECT_DOUBLE_EQ(signal.getMotorRpm(), 15000.5);
}

TEST(VehicleSignalTest, GearSelectorStoresValidValues) {
    VehicleSignal signalP(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "P");
    VehicleSignal signalR(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "R");
    VehicleSignal signalN(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "N");
    VehicleSignal signalD(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "D");
    VehicleSignal signalS(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, "S");

    EXPECT_EQ(signalP.getGearSelector(), "P");
    EXPECT_EQ(signalR.getGearSelector(), "R");
    EXPECT_EQ(signalN.getGearSelector(), "N");
    EXPECT_EQ(signalD.getGearSelector(), "D");
    EXPECT_EQ(signalS.getGearSelector(), "S");
}

TEST(VehicleSignalTest, GearSelectorStoresEmptyStringByDefault) {
    VehicleSignal signal(0, 0, 0, 0, 0);
    EXPECT_TRUE(signal.getGearSelector().empty());
}

TEST(VehicleSignalTest, GearSelectorMovesInputCorrectly) {
    std::string gear = "D";
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, std::move(gear));
    EXPECT_EQ(signal.getGearSelector(), "D");
}

TEST(VehicleSignalTest, MotorTorqueNmIsClampedToMax7500) {
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 8000.0, "");
    EXPECT_DOUBLE_EQ(signal.getMotorTorqueNm(), 7500.0);
}

TEST(VehicleSignalTest, MotorTorqueNmIsClampedToMinNegative7500) {
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, -8000.0, "");
    EXPECT_DOUBLE_EQ(signal.getMotorTorqueNm(), -7500.0);
}

TEST(VehicleSignalTest, MotorTorqueNmAcceptsValidRange) {
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 375.5, "");
    EXPECT_DOUBLE_EQ(signal.getMotorTorqueNm(), 375.5);
}

TEST(VehicleSignalTest, MotorTorqueNmAcceptsNegativeValues) {
    VehicleSignal signal(0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, -250.5, "");
    EXPECT_DOUBLE_EQ(signal.getMotorTorqueNm(), -250.5);
}

TEST(VehicleSignalTest, MotorTorqueNmIsZeroByDefault) {
    VehicleSignal signal(0, 0, 0, 0, 0);
    EXPECT_DOUBLE_EQ(signal.getMotorTorqueNm(), 0.0);
}

TEST(VehicleSignalTest, MotorRpmIsZeroByDefault) {
    VehicleSignal signal(0, 0, 0, 0, 0);
    EXPECT_DOUBLE_EQ(signal.getMotorRpm(), 0.0);
}

TEST(VehicleSignalTest, EqualityIncludesNewFields) {
    VehicleSignal a(50.0, 100.0, 0.5, 25.0, 12345ULL, 0.0, 3500.0, 0.0, 0.0, 150.0, "D");
    VehicleSignal b(50.0, 100.0, 0.5, 25.0, 12345ULL, 0.0, 3500.0, 0.0, 0.0, 150.0, "D");
    VehicleSignal c(50.0, 100.0, 0.5, 25.0, 12345ULL, 0.0, 4000.0, 0.0, 0.0, 150.0, "D"); // different rpm
    VehicleSignal d(50.0, 100.0, 0.5, 25.0, 12345ULL, 0.0, 3500.0, 0.0, 0.0, 150.0, "N"); // different gear
    VehicleSignal e(50.0, 100.0, 0.5, 25.0, 12345ULL, 0.0, 3500.0, 0.0, 0.0, 200.0, "D"); // different torque

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST(VehicleSignalTest, InequalityIncludesNewFields) {
    VehicleSignal a(50.0, 100.0, 0.5, 25.0, 12345ULL, 0.0, 3500.0, 0.0, 0.0, 150.0, "D");
    VehicleSignal b(50.0, 100.0, 0.5, 25.0, 12345ULL, 0.0, 3500.0, 0.0, 0.0, 150.0, "N");

    EXPECT_NE(a, b);
}

TEST(VehicleSignalTest, AllFieldsConstructCorrectly) {
    VehicleSignal signal(50.0, 100.0, 0.5, 25.0, 123456789ULL, 0.0, 5000.0, 0.0, 0.0, 300.0, "D");

    EXPECT_DOUBLE_EQ(signal.getThrottlePercent(), 50.0);
    EXPECT_DOUBLE_EQ(signal.getSpeedKmh(), 100.0);
    EXPECT_DOUBLE_EQ(signal.getAccelerationG(), 0.5);
    EXPECT_DOUBLE_EQ(signal.getBrakePercent(), 25.0);
    EXPECT_EQ(signal.getTimestampUtcMs(), 123456789ULL);
    EXPECT_DOUBLE_EQ(signal.getMotorRpm(), 5000.0);
    EXPECT_EQ(signal.getGearSelector(), "D");
    EXPECT_DOUBLE_EQ(signal.getMotorTorqueNm(), 300.0);
}

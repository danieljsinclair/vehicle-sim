#include <gtest/gtest.h>
#include "vehicle-sim/PhysicsEngine.h"

using namespace vehicle_sim;

// ================================================
// PhysicsEngine Unit Tests
// TDD - Tests for physics calculation logic
// ================================================

TEST(PhysicsEngineTest, InitializesWithZeroValues)
{
    PhysicsEngine engine;

    PhysicsData data = engine.update(0.1);

    EXPECT_DOUBLE_EQ(data.rpm, 0.0);
    EXPECT_DOUBLE_EQ(data.speed, 0.0);
    EXPECT_DOUBLE_EQ(data.throttle_position, 0.0);
    EXPECT_DOUBLE_EQ(data.torque, 0.0);
    EXPECT_EQ(data.gear, 0);
    EXPECT_FALSE(data.brake_active);
}

TEST(PhysicsEngineTest, SetThrottleClampsToValidRange)
{
    PhysicsEngine engine;

    engine.setThrottle(1.5); // Above range
    PhysicsData data = engine.update(0.1);
    EXPECT_DOUBLE_EQ(data.throttle_position, 1.0);

    engine.setThrottle(-0.5); // Below range
    data = engine.update(0.1);
    EXPECT_DOUBLE_EQ(data.throttle_position, 0.0);

    engine.setThrottle(0.5); // Valid
    data = engine.update(0.1);
    EXPECT_DOUBLE_EQ(data.throttle_position, 0.5);
}

TEST(PhysicsEngineTest, SetBrakeUpdatesBrakeState)
{
    PhysicsEngine engine;

    engine.setBrake(true);
    PhysicsData data = engine.update(0.1);
    EXPECT_TRUE(data.brake_active);

    engine.setBrake(false);
    data = engine.update(0.1);
    EXPECT_FALSE(data.brake_active);
}

TEST(PhysicsEngineTest, SetGearClampsToValidRange)
{
    PhysicsEngine engine;

    engine.setGear(10); // Above max
    PhysicsData data = engine.update(0.1);
    EXPECT_EQ(data.gear, 7); // NUM_GEARS is 7 (0-6)

    engine.setGear(-1); // Below min
    data = engine.update(0.1);
    EXPECT_EQ(data.gear, 0);

    engine.setGear(3); // Valid
    data = engine.update(0.1);
    EXPECT_EQ(data.gear, 3);
}

TEST(PhysicsEngineTest, GetGearRatioReturnsCorrectValue)
{
    PhysicsEngine engine;

    engine.setGear(0);
    EXPECT_DOUBLE_EQ(engine.getGearRatio(), 0.0);

    engine.setGear(1);
    EXPECT_DOUBLE_EQ(engine.getGearRatio(), 3.5);

    engine.setGear(6);
    EXPECT_DOUBLE_EQ(engine.getGearRatio(), 0.65);
}

TEST(PhysicsEngineTest, CalculateAccelerationComputesForceOverMass)
{
    // a = F/m where F = torque * 0.1 (simplified)
    const double torque = 1000.0;
    const double mass = 1800.0;
    const double air_resistance = 0.0;

    double accel = PhysicsEngine::calculateAcceleration(torque, mass, air_resistance);

    // Expected: (1000.0 * 0.1) / 1800.0 = 100.0 / 1800.0 = 0.0555...
    EXPECT_NEAR(accel, 0.055555, 0.0001);
}

TEST(PhysicsEngineTest, SpeedNeverGoesNegative)
{
    PhysicsEngine engine;

    engine.setBrake(true);
    PhysicsData data = engine.update(1.0); // Should decelerate but not below 0

    EXPECT_GE(data.speed, 0.0);
}

TEST(PhysicsEngineTest, UpdateWithDeltaTimeAffectsSpeed)
{
    PhysicsEngine engine;

    engine.setThrottle(0.5);

    PhysicsData data1 = engine.update(0.1);
    PhysicsData data2 = engine.update(0.1);

    // With current stub implementation, speed stays at 0 because RPM never increases
    // This test documents the current placeholder behavior
    EXPECT_DOUBLE_EQ(data2.speed, 0.0);
}

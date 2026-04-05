#include "vehicle-sim/PhysicsEngine.h"
#include <algorithm>

namespace vehicle_sim {

PhysicsEngine::PhysicsEngine()
    : current_rpm(0.0)
    , current_speed(0.0)
    , throttle_position(0.0)
    , brake_active(false)
    , current_gear(0) {
}

PhysicsEngine::~PhysicsEngine() {
}

void PhysicsEngine::setThrottle(double position) {
    throttle_position = std::clamp(position, 0.0, 1.0);
}

void PhysicsEngine::setBrake(bool active) {
    brake_active = active;
}

void PhysicsEngine::setGear(int gear) {
    current_gear = std::clamp(gear, 0, NUM_GEARS);
}

PhysicsData PhysicsEngine::update(double delta_time) {
    PhysicsData data{};

    // Simple physics simulation
    // This is a placeholder - implement actual physics here

    double gear_ratio = getGearRatio();
    double wheel_rpm = current_rpm / (gear_ratio * 3.6); // Simplified

    // Update speed based on throttle and gear
    if (!brake_active && throttle_position > 0.0) {
        double accel = calculateAcceleration(
            current_rpm * 0.1, // Simplified torque calculation
            MASS_KG,
            0.0
        );
        current_speed += accel * delta_time * 3.6; // Convert to km/h
    }

    if (brake_active) {
        current_speed -= 50.0 * delta_time; // Deceleration
    }

    current_speed = std::max(0.0, current_speed);

    // Populate return data
    data.timestamp = 0.0; // Will be set by caller
    data.rpm = current_rpm;
    data.speed = current_speed;
    data.throttle_position = throttle_position;
    data.torque = current_rpm * 0.1; // Placeholder
    data.acceleration = 0.0; // Calculated above
    data.gear = current_gear;
    data.brake_active = brake_active;

    return data;
}

double PhysicsEngine::getGearRatio() const {
    if (current_gear < 0 || current_gear > NUM_GEARS) {
        return 1.0;
    }
    return gear_ratios[current_gear];
}

double PhysicsEngine::calculateAcceleration(double torque, double mass, double air_resistance) {
    // F = ma, so a = F/m
    // Force from torque depends on wheel radius, simplified here
    double force = torque * 0.1; // Placeholder conversion
    return force / mass;
}

} // namespace vehicle_sim

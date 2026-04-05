#pragma once

#include <cmath>
#include <cstdint>

namespace vehicle_sim {

struct PhysicsData {
    double timestamp;           // seconds
    double rpm;                 // engine RPM
    double speed;               // km/h or mph (configurable)
    double throttle_position;   // 0.0 to 1.0
    double torque;              // Nm or lb-ft
    double acceleration;        // m/s² or g-force
    double gear;                // current gear (0 = neutral)
    bool   brake_active;        // brake pedal status
};

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    // Set current vehicle parameters
    void setThrottle(double position);  // 0.0 - 1.0
    void setBrake(bool active);
    void setGear(int gear);

    // Update physics simulation
    // delta_time: time since last update in seconds
    PhysicsData update(double delta_time);

    // Get current gear ratio for the current gear
    double getGearRatio() const;

    // Calculate acceleration from torque and mass
    static double calculateAcceleration(double torque, double mass, double air_resistance);

private:
    // Internal state
    double current_rpm;
    double current_speed;
    double throttle_position;
    bool brake_active;
    int current_gear;

    // Vehicle specifications (would be loaded from config)
    static constexpr double MASS_KG = 1800.0;
    static constexpr double WHEEL_DIAMETER_M = 0.67;
    static constexpr int NUM_GEARS = 7;  // Neutral + 6 forward gears
    double gear_ratios[NUM_GEARS] = { 0.0, 3.5, 2.1, 1.4, 1.0, 0.8, 0.65 };
};

} // namespace vehicle_sim

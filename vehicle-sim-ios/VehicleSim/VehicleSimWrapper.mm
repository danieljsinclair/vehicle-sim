#import "VehicleSimWrapper.h"
#include <cmath>
#include <chrono>

@interface VehicleSimWrapper ()
@property (nonatomic) BOOL running;
@property (nonatomic) int tick;
@end

@implementation VehicleSimWrapper

- (instancetype)init {
    self = [super init];
    if (self) {
        _running = false;
        _tick = 0;
    }
    return self;
}

- (void)start {
    if (_running) return;
    _running = true;
}

- (void)stop {
    if (!_running) return;
    _running = false;
}

- (TelemetryData *)getTelemetry {
    if (!_running) return nil;

    _tick++;

    // Simulate a driving scenario with smooth value transitions
    double t = _tick * 0.1;  // time in seconds

    // Throttle: oscillates between 0-60% with sine wave (simulating pedal input)
    double throttle = 30.0 + 25.0 * sin(t * 0.3) + 5.0 * sin(t * 1.7);

    // Speed: follows throttle with lag (vehicle accelerates/decelerates)
    static double speed = 0.0;
    double targetSpeed = throttle * 1.8; // throttle maps roughly to speed
    speed += (targetSpeed - speed) * 0.05; // smooth interpolation

    // RPM: correlates with speed and throttle
    double rpm = 800 + speed * 30 + throttle * 10 + 5.0 * sin(t * 2.1);

    // Brake: occasionally pulses
    double brake = (fmod(t, 8.0) > 7.0) ? 20.0 + 10.0 * sin(t * 5.0) : 0.0;

    // Acceleration: derivative of speed
    static double lastSpeed = 0.0;
    double acceleration = (speed - lastSpeed) * 2.0;
    lastSpeed = speed;

    // Clamp values
    throttle = fmax(0.0, fmin(100.0, throttle));
    speed = fmax(0.0, fmin(200.0, speed));
    rpm = fmax(600.0, fmin(8000.0, rpm));
    brake = fmax(0.0, fmin(100.0, brake));
    acceleration = fmax(-1.5, fmin(1.5, acceleration));

    // Gear: simple mapping from speed
    int gear = 0;
    if (speed > 5) gear = 1;
    if (speed > 20) gear = 2;
    if (speed > 40) gear = 3;
    if (speed > 60) gear = 4;
    if (speed > 90) gear = 5;
    if (speed > 120) gear = 6;

    return [[TelemetryData alloc] initWithTimestamp:NSDate.timeIntervalSinceReferenceDate
                                                              rpm:rpm
                                                        speedKmh:speed
                                                throttlePercent:throttle
                                                   brakePercent:brake
                                                           gear:gear
                                                         torque:rpm * 0.12
                                                accelerationG:acceleration];
}

- (BOOL)isRunning {
    return _running;
}

@end

@implementation TelemetryData

- (instancetype)initWithTimestamp:(NSTimeInterval)timestamp
                              rpm:(double)rpm
                        speedKmh:(double)speedKmh
                throttlePercent:(double)throttlePercent
                   brakePercent:(double)brakePercent
                           gear:(NSInteger)gear
                         torque:(double)torque
                accelerationG:(double)accelerationG {
    self = [super init];
    if (self) {
        _timestamp = timestamp;
        _rpm = rpm;
        _speedKmh = speedKmh;
        _throttlePercent = throttlePercent;
        _brakePercent = brakePercent;
        _gear = gear;
        _torque = torque;
        _accelerationG = accelerationG;
    }
    return self;
}

@end

#pragma once

#include <cstdint>

namespace vehicle_sim::domain {

// OBD2 PID scaling formulas per SAE J1979.
// Single source of truth — use these everywhere OBD2 data is decoded.

// SAE J1979 protocol constants — every value here is dictated by the spec.
//
//   OBD2_MAX_BYTE (255.0):
//     Maximum value of an unsigned 8-bit byte (2^8 - 1).
//     SAE J1979 uses this as the denominator for single-byte percentage PIDs
//     (04 calculated load, 11 throttle position, 2C/2E/2F/45/47-4C various
//     throttle/fuel percentages):  physical = (A / 255) * 100.
//
//   OBD2_PERCENT_SCALE (100.0):
//     Converts the 0-1 normalised ratio to 0-100%.
//
//   OBD2_MULTI_BYTE_SCALE (256.0):
//     Byte-shift multiplier for combining two bytes into a 16-bit word.
//     Equals 2^8 (i.e. A << 8 | B).  Used in RPM (PID 0C), MAF (PID 10),
//     fuel pressure (PID 22/23), run time (PID 1F), and many others.
//
//   OBD2_RPM_DIVISOR (4.0):
//     SAE J1979 PID 0C: RPM = ((A * 256) + B) / 4.
//     Dividing the 16-bit value by 4 gives 0.25 RPM resolution,
//     yielding a range of 0 – 16,383.75 RPM — sufficient for any engine.
//
//   OBD2_TEMP_OFFSET (40.0):
//     SAE J1979 PIDs 05 (coolant), 0F (intake air), 46 (ambient):
//     physical = A - 40.
//     Subtracting 40 from the unsigned byte (0-255) extends the measurable
//     range to -40°C to +215°C.  The -40°C lower bound covers cold-start
//     conditions in any climate.  (The coincidence with the °C/°F crossover
//     at -40 is just that — a coincidence.)

static constexpr double OBD2_MAX_BYTE         = 255.0;   // 2^8 - 1
static constexpr double OBD2_PERCENT_SCALE    = 100.0;   // (A / 255) * 100
static constexpr double OBD2_MULTI_BYTE_SCALE = 256.0;   // A << 8  (2^8)
static constexpr double OBD2_RPM_DIVISOR      = 4.0;     // ((A*256)+B) / 4  → 0.25 RPM resolution
static constexpr double OBD2_TEMP_OFFSET      = 40.0;    // A - 40  → -40°C to +215°C (standard approach to range 0-255 into usable temp range by subtracting 40 apparently since -40°C is the coldest temp likely to be encountered in automotive contexts)

// Single byte → percentage: (A / 255) * 100   (PIDs 04, 11, 2C, 2E, 2F, 45, 47-4C)
inline double obd2BytePercent(std::uint8_t byte) noexcept {
    return (static_cast<double>(byte) / OBD2_MAX_BYTE) * OBD2_PERCENT_SCALE;
}

// Two bytes → RPM: ((A * 256) + B) / 4   (PID 0C)
inline double obd2WordRPM(std::uint8_t msb, std::uint8_t lsb) noexcept {
    return (static_cast<double>(msb) * OBD2_MULTI_BYTE_SCALE
          + static_cast<double>(lsb)) / OBD2_RPM_DIVISOR;
}

// Single byte → Celsius: A - 40   (PIDs 05, 0F, 46)
inline double obd2TempCelsius(std::uint8_t byte) noexcept {
    return static_cast<double>(byte) - OBD2_TEMP_OFFSET;
}

// Raw byte value (speed, etc.): A as-is   (PIDs 0D, 33, 50)
inline double obd2RawValue(std::uint8_t byte) noexcept {
    return static_cast<double>(byte);
}

} // namespace vehicle_sim::domain

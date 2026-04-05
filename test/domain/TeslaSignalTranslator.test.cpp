#include <gtest/gtest.h>
#include "vehicle-sim/domain/TeslaSignalTranslator.h"
#include "vehicle-sim/domain/VehicleSignal.h"

using namespace vehicle_sim::domain;

// ================================================
// TeslaSignalTranslator Unit Tests
// TDD - Tests for Tesla-specific BLE data translation
// ================================================

TEST(TeslaSignalTranslatorTest, TranslatesValidTeslaPacket)
{
    TeslaSignalTranslator translator;

    // Mock Tesla BLE packet format:
    // [0xAA][0x55][0x05][throttle][speed_lo][speed_hi][accel][brake][checksum]
    // throttle: 0-100 (percentage)
    // speed: uint16_t km/h (little endian)
    // accel: int8_t * 10 (acceleration in G)
    // brake: 0-100 (percentage)
    std::vector<std::uint8_t> validPacket = {
        0xAA, 0x55,             // Header
        0x05,                   // Payload length (5 bytes: throttle, speed_lo, speed_hi, accel, brake)
        50,                     // Throttle 50%
        100, 0,                 // Speed 100 km/h (little endian)
        5,                      // Acceleration 0.5 G
        25,                     // Brake 25%
        0x00                    // Simple checksum (sum & 0xFF)
    };

    // Calculate checksum
    std::uint8_t checksum = 0;
    for (size_t i = 0; i < validPacket.size() - 1; ++i) {
        checksum += validPacket[i];
    }
    validPacket[validPacket.size() - 1] = checksum;

    EXPECT_TRUE(translator.isValidPacket(validPacket));

    auto result = translator.translate(validPacket);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getThrottlePercent(), 50.0);
    EXPECT_DOUBLE_EQ(result->getSpeedKmh(), 100.0);
    EXPECT_DOUBLE_EQ(result->getAccelerationG(), 0.5);
    EXPECT_DOUBLE_EQ(result->getBrakePercent(), 25.0);
}

TEST(TeslaSignalTranslatorTest, RejectsInvalidHeader)
{
    TeslaSignalTranslator translator;
    std::vector<std::uint8_t> invalidPacket = {
        0xFF, 0xFF, 0x05, 50, 100, 0, 5, 25, 0x00
    };

    EXPECT_FALSE(translator.isValidPacket(invalidPacket));
    EXPECT_FALSE(translator.translate(invalidPacket).has_value());
}

TEST(TeslaSignalTranslatorTest, RejectsWrongLength)
{
    TeslaSignalTranslator translator;
    std::vector<std::uint8_t> shortPacket = {
        0xAA, 0x55, 0x02, 50, 0x00
    };

    EXPECT_FALSE(translator.isValidPacket(shortPacket));
    EXPECT_FALSE(translator.translate(shortPacket).has_value());
}

TEST(TeslaSignalTranslatorTest, RejectsBadChecksum)
{
    TeslaSignalTranslator translator;
    std::vector<std::uint8_t> badChecksumPacket = {
        0xAA, 0x55, 0x05, 50, 100, 0, 5, 25, 0xFF  // Wrong checksum
    };

    EXPECT_FALSE(translator.isValidPacket(badChecksumPacket));
    EXPECT_FALSE(translator.translate(badChecksumPacket).has_value());
}

TEST(TeslaSignalTranslatorTest, ClampsTranslatedValues)
{
    TeslaSignalTranslator translator;

    std::vector<std::uint8_t> outOfRangePacket = {
        0xAA, 0x55, 0x05,
        200,                    // Throttle 200% (should clamp to 100)
        255, 255,               // Speed 65535 km/h (should clamp to 300)
        127,                    // Acceleration 12.7 G (should clamp to 5.0)
        150,                    // Brake 150% (should clamp to 100)
        0x00
    };

    std::uint8_t checksum = 0;
    for (size_t i = 0; i < outOfRangePacket.size() - 1; ++i) {
        checksum += outOfRangePacket[i];
    }
    outOfRangePacket[outOfRangePacket.size() - 1] = checksum;

    auto result = translator.translate(outOfRangePacket);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getThrottlePercent(), 100.0);
    EXPECT_DOUBLE_EQ(result->getSpeedKmh(), 300.0);
    EXPECT_DOUBLE_EQ(result->getAccelerationG(), 5.0);
    EXPECT_DOUBLE_EQ(result->getBrakePercent(), 100.0);
}

TEST(TeslaSignalTranslatorTest, HandlesNegativeAcceleration)
{
    TeslaSignalTranslator translator;

    std::vector<std::uint8_t> brakingPacket = {
        0xAA, 0x55, 0x05,
        0,                      // Throttle 0%
        50, 0,                  // Speed 50 km/h
        128,                    // Acceleration -12.8 G (should clamp to -5.0)
        80,                     // Brake 80%
        0x00
    };

    std::uint8_t checksum = 0;
    for (size_t i = 0; i < brakingPacket.size() - 1; ++i) {
        checksum += brakingPacket[i];
    }
    brakingPacket[brakingPacket.size() - 1] = checksum;

    auto result = translator.translate(brakingPacket);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getAccelerationG(), -5.0);
}

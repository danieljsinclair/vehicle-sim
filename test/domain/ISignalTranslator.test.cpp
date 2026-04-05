#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "vehicle-sim/domain/ISignalTranslator.h"
#include "vehicle-sim/domain/VehicleSignal.h"

using namespace vehicle_sim::domain;
using testing::_;
using testing::Return;

// ================================================
// ISignalTranslator Interface Tests
// TDD - Tests verify interface contract
// ================================================

class MockSignalTranslator : public ISignalTranslator {
public:
    MOCK_METHOD(std::optional<VehicleSignal>, translate, (const std::vector<std::uint8_t>& rawData), (const, noexcept, override));
    MOCK_METHOD(bool, isValidPacket, (const std::vector<std::uint8_t>& rawData), (const, noexcept, override));
};

TEST(ISignalTranslatorTest, InterfaceHasCorrectMethods)
{
    // Compile time test: interface methods exist and are callable
    MockSignalTranslator mock;
    std::vector<std::uint8_t> dummyData = {0x01, 0x02, 0x03};

    // These calls compile, verifying the interface contract
    (void)mock.translate(dummyData);
    (void)mock.isValidPacket(dummyData);

    SUCCEED();
}

TEST(ISignalTranslatorTest, TranslateReturnsOptionalVehicleSignal)
{
    MockSignalTranslator mock;
    std::vector<std::uint8_t> validData = {0xAA, 0x55};

    EXPECT_CALL(mock, translate(validData))
        .WillOnce(Return(VehicleSignal(50.0, 100.0, 0.5, 0.0, 12345)));

    auto result = mock.translate(validData);
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getThrottlePercent(), 50.0);
}

TEST(ISignalTranslatorTest, TranslateReturnsEmptyOptionalOnFailure)
{
    MockSignalTranslator mock;
    std::vector<std::uint8_t> invalidData = {0xFF};

    EXPECT_CALL(mock, translate(invalidData))
        .WillOnce(Return(std::nullopt));

    auto result = mock.translate(invalidData);
    EXPECT_FALSE(result.has_value());
}

TEST(ISignalTranslatorTest, IsValidPacketValidatesData)
{
    MockSignalTranslator mock;
    std::vector<std::uint8_t> validData = {0xAA, 0x55};
    std::vector<std::uint8_t> invalidData = {0x00};

    EXPECT_CALL(mock, isValidPacket(validData))
        .WillOnce(Return(true));
    EXPECT_CALL(mock, isValidPacket(invalidData))
        .WillOnce(Return(false));

    EXPECT_TRUE(mock.isValidPacket(validData));
    EXPECT_FALSE(mock.isValidPacket(invalidData));
}

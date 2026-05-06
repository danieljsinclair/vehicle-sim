#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "vehicle-sim/BLEManager.h"
#include "vehicle-sim/ble/BLEManagerBase.h"

using namespace vehicle_sim;
using testing::_;
using testing::Return;
using testing::SaveArg;
using testing::Eq;
using testing::Field;

// Mock BLE platform for testing - inherits from BLEManagerBase
class MockBLEManagerBase : public BLEManagerBase {
public:
    MOCK_METHOD(std::vector<BLEDeviceInfo>, scanForDevices, (int timeout_seconds), (override));
    MOCK_METHOD(bool, connect, (const std::string& device_identifier), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(void, send, (const std::vector<uint8_t>& data), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
    MOCK_METHOD(std::string, getConnectedDeviceId, (), (const, override));
    MOCK_METHOD(void, setDeviceFoundCallback, (DeviceCallback callback), (override));
    MOCK_METHOD(void, setDataReceivedCallback, (DataCallback callback), (override));
};

// ================================================
// BLEManager Unit Tests
// TDD - Tests using dependency injection with mocks
// ================================================

TEST(BLEManagerTest, ScansForDevicesViaPlatform)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    EXPECT_CALL(*mockPlatform, scanForDevices(5))
        .WillOnce(Return(std::vector<BLEDeviceInfo>{
            {"addr1", "Device 1", false, -50},
            {"addr2", "Device 2", false, -60}
        }));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    auto devices = manager.scanForDevices(5);
    EXPECT_EQ(devices.size(), 2);
    EXPECT_EQ(devices[0].name, "Device 1");
    EXPECT_EQ(devices[1].name, "Device 2");
}

TEST(BLEManagerTest, ConnectsToDeviceViaPlatform)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    EXPECT_CALL(*mockPlatform, connect(Eq("test-device")))
        .WillOnce(Return(true));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    bool result = manager.connect("test-device");
    EXPECT_TRUE(result);
}

TEST(BLEManagerTest, ReportsConnectionFailure)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    EXPECT_CALL(*mockPlatform, connect(Eq("invalid-device")))
        .WillOnce(Return(false));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    bool result = manager.connect("invalid-device");
    EXPECT_FALSE(result);
}

TEST(BLEManagerTest, DisconnectsViaPlatform)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    EXPECT_CALL(*mockPlatform, disconnect()).Times(1);

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    manager.disconnect();
}

TEST(BLEManagerTest, ReportsConnectionStatus)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    EXPECT_CALL(*mockPlatform, isConnected())
        .WillOnce(Return(true));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    EXPECT_TRUE(manager.isConnected());
}

TEST(BLEManagerTest, ReturnsConnectedDeviceId)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    EXPECT_CALL(*mockPlatform, getConnectedDeviceId())
        .WillOnce(Return(std::string("device-123")));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    std::string deviceId = manager.getConnectedDeviceId();
    EXPECT_EQ(deviceId, "device-123");
}

TEST(BLEManagerTest, ReturnsEmptyDeviceIdWhenNotConnected)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    EXPECT_CALL(*mockPlatform, getConnectedDeviceId())
        .WillOnce(Return(std::string()));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    std::string deviceId = manager.getConnectedDeviceId();
    EXPECT_TRUE(deviceId.empty());
}

TEST(BLEManagerTest, ForwardsDeviceFoundCallback)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    BLEManager::DeviceCallback capturedCallback;

    EXPECT_CALL(*mockPlatform, setDeviceFoundCallback(_))
        .WillOnce(SaveArg<0>(&capturedCallback));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    bool callbackInvoked = false;
    manager.onDeviceFound([&callbackInvoked](const BLEDeviceInfo& device) {
        callbackInvoked = true;
        EXPECT_EQ(device.name, "Test Device");
    });

    // Simulate the platform invoking the callback
    capturedCallback({"addr", "Test Device", false, -50});

    EXPECT_TRUE(callbackInvoked);
}

TEST(BLEManagerTest, ForwardsDataReceivedCallback)
{
    auto mockPlatform = std::make_unique<MockBLEManagerBase>();
    BLEManager::DataCallback capturedCallback;

    EXPECT_CALL(*mockPlatform, setDataReceivedCallback(_))
        .WillOnce(SaveArg<0>(&capturedCallback));

    BLEManager manager;
    manager.setPlatform(std::move(mockPlatform));

    bool callbackInvoked = false;
    std::vector<uint8_t> expectedData = {0xAA, 0x55, 0x01, 0x02, 0x03};

    manager.onDataReceived([&callbackInvoked, &expectedData](const std::vector<uint8_t>& data) {
        callbackInvoked = true;
        EXPECT_EQ(data, expectedData);
    });

    // Simulate the platform invoking the callback
    capturedCallback(expectedData);

    EXPECT_TRUE(callbackInvoked);
}

TEST(BLEManagerTest, HandlesNullPlatformGracefully)
{
    BLEManager manager;
    manager.setPlatform(nullptr);

    // Should not crash and return safe defaults
    EXPECT_FALSE(manager.isConnected());
    EXPECT_TRUE(manager.getConnectedDeviceId().empty());
    EXPECT_EQ(manager.scanForDevices(5).size(), 0);
    EXPECT_FALSE(manager.connect("device"));

    // These should not crash either
    manager.disconnect();
    manager.onDeviceFound([](const BLEDeviceInfo&) {});
    manager.onDataReceived([](const std::vector<uint8_t>&) {});
}

// ================================================
// BLEManagerBase OBD2 Helper Tests
// ================================================

TEST(BLEManagerBaseTest, ParsesASCIIResponseToBinary)
{
    // Create a concrete test subclass since BLEManagerBase is abstract
    class TestBLEManager : public BLEManagerBase {
    public:
        std::vector<BLEDeviceInfo> scanForDevices(int) override { return {}; }
        bool connect(const std::string&) override { return false; }
        void disconnect() override {}
        void send(const std::vector<uint8_t>&) override {}
        bool isConnected() const override { return false; }
        std::string getConnectedDeviceId() const override { return {}; }

        // Expose private method for testing
        std::vector<uint8_t> testParseASCIIResponseToBinary(const std::vector<uint8_t>& asciiData) {
            return parseASCIIResponseToBinary(asciiData);
        }
    };

    TestBLEManager manager;

    // Simulate ASCII hex response from ELM327: "41 0C 1A F8\r"
    std::vector<uint8_t> asciiData = {'4', '1', ' ', '0', 'C', ' ', '1', 'A', ' ', 'F', '8', '\r'};
    auto result = manager.testParseASCIIResponseToBinary(asciiData);

    // Should parse to binary [0x41, 0x0C, 0x1A, 0xF8]
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 0x41);
    EXPECT_EQ(result[1], 0x0C);
    EXPECT_EQ(result[2], 0x1A);
    EXPECT_EQ(result[3], 0xF8);
}

TEST(BLEManagerBaseTest, SkipsPromptAndEcho)
{
    class TestBLEManager : public BLEManagerBase {
    public:
        std::vector<BLEDeviceInfo> scanForDevices(int) override { return {}; }
        bool connect(const std::string&) override { return false; }
        void disconnect() override {}
        void send(const std::vector<uint8_t>&) override {}
        bool isConnected() const override { return false; }
        std::string getConnectedDeviceId() const override { return {}; }

        std::vector<uint8_t> testParseASCIIResponseToBinary(const std::vector<uint8_t>& asciiData) {
            return parseASCIIResponseToBinary(asciiData);
        }
    };

    TestBLEManager manager;

    // ELM327 prompt character only
    std::vector<uint8_t> prompt = {'>'};
    auto result = manager.testParseASCIIResponseToBinary(prompt);
    EXPECT_TRUE(result.empty());

    // Error message
    std::vector<uint8_t> error = {'N', 'O', ' ', 'D', 'A', 'T', 'A', '\r'};
    result = manager.testParseASCIIResponseToBinary(error);
    EXPECT_TRUE(result.empty());
}

TEST(BLEManagerBaseTest, BuildsAndSendsOBD2QueryWithELM327Encoding)
{
    class TestBLEManager : public BLEManagerBase {
    public:
        std::vector<BLEDeviceInfo> scanForDevices(int) override { return {}; }
        bool connect(const std::string&) override { return false; }
        void disconnect() override {}
        std::vector<uint8_t> lastSent;

        void send(const std::vector<uint8_t>& data) override {
            lastSent = data;
        }

        bool isConnected() const override { return false; }
        std::string getConnectedDeviceId() const override { return {}; }
    };

    TestBLEManager manager;

    // Query PID 0x0C (Engine RPM)
    manager.queryPID(0x0C);

    // Should send ASCII "01 0C\r" as bytes
    std::string expected = "01 0C\r";
    EXPECT_EQ(manager.lastSent.size(), expected.size());

    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(manager.lastSent[i], static_cast<uint8_t>(expected[i]));
    }
}

TEST(BLEManagerBaseTest, SignalQualityConversion)
{
    // Test various RSSI values
    EXPECT_EQ(BLEManagerBase::signalQuality(-45), "Excellent");
    EXPECT_EQ(BLEManagerBase::signalQuality(-50), "Excellent");
    EXPECT_EQ(BLEManagerBase::signalQuality(-55), "Good");
    EXPECT_EQ(BLEManagerBase::signalQuality(-65), "Good");
    EXPECT_EQ(BLEManagerBase::signalQuality(-70), "Fair");
    EXPECT_EQ(BLEManagerBase::signalQuality(-75), "Fair");
    EXPECT_EQ(BLEManagerBase::signalQuality(-90), "Poor");
}
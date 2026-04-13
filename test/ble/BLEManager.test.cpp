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

TEST(BLEManagerBaseTest, BuildsOBD2QueryCorrectly)
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
    };

    TestBLEManager manager;

    // Test throttle position query (PID 0x11)
    auto cmd = manager.buildOBD2Query(0x11);
    EXPECT_EQ(cmd.size(), 2);
    EXPECT_EQ(cmd[0], 0x01);  // Mode 01
    EXPECT_EQ(cmd[1], 0x11);  // Throttle PID

    // Test vehicle speed query (PID 0x0D)
    auto cmd2 = manager.buildOBD2Query(0x0D);
    EXPECT_EQ(cmd2.size(), 2);
    EXPECT_EQ(cmd2[0], 0x01);
    EXPECT_EQ(cmd2[1], 0x0D);
}

TEST(BLEManagerBaseTest, ParsesOBD2ThrottleResponse)
{
    class TestBLEManager : public BLEManagerBase {
    public:
        std::vector<BLEDeviceInfo> scanForDevices(int) override { return {}; }
        bool connect(const std::string&) override { return false; }
        void disconnect() override {}
        void send(const std::vector<uint8_t>&) override {}
        bool isConnected() const override { return false; }
        std::string getConnectedDeviceId() const override { return {}; }
    };

    TestBLEManager manager;

    // Simulate OBD2 response: Mode 0x41, PID 0x11, Data 0x80 (50% throttle)
    std::vector<uint8_t> response = {0x41, 0x11, 0x80};

    auto result = manager.parseOBD2Response(response);
    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.pid, 0x11);
    EXPECT_TRUE(result.value.has_value());
    EXPECT_NEAR(*result.value, 50.0, 1.0);  // ~50% throttle
}

TEST(BLEManagerBaseTest, ParsesOBD2SpeedResponse)
{
    class TestBLEManager : public BLEManagerBase {
    public:
        std::vector<BLEDeviceInfo> scanForDevices(int) override { return {}; }
        bool connect(const std::string&) override { return false; }
        void disconnect() override {}
        void send(const std::vector<uint8_t>&) override {}
        bool isConnected() const override { return false; }
        std::string getConnectedDeviceId() const override { return {}; }
    };

    TestBLEManager manager;

    // Simulate OBD2 response: Mode 0x41, PID 0x0D, Data 0x50 (80 km/h)
    std::vector<uint8_t> response = {0x41, 0x0D, 0x50};

    auto result = manager.parseOBD2Response(response);
    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.pid, 0x0D);
    EXPECT_TRUE(result.value.has_value());
    EXPECT_EQ(*result.value, 80.0);  // 80 km/h
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
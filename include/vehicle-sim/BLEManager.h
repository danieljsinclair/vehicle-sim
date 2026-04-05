#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace vehicle_sim {

struct BLEDeviceInfo {
    std::string address;
    std::string name;
    bool isConnected;
};

class BLEManager {
public:
    using DeviceCallback = std::function<void(const BLEDeviceInfo& device)>;
    using DataCallback = std::function<void(const std::vector<uint8_t>& data)>;

    BLEManager();
    ~BLEManager();

    // Scan for BLE devices
    std::vector<BLEDeviceInfo> scanForDevices(int timeout_seconds = 5);

    // Connect to a specific device by address
    bool connect(const std::string& device_address);

    // Disconnect from current device
    void disconnect();

    // Subscribe to device discovery events
    void onDeviceFound(DeviceCallback callback);

    // Subscribe to raw BLE data
    void onDataReceived(DataCallback callback);

    // Check connection status
    bool isConnected() const;

private:
    bool connected_;
    DeviceCallback device_callback_;
    DataCallback data_callback_;
};

} // namespace vehicle_sim

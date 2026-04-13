#pragma once

#include <string>
#include <cstdint>

namespace vehicle_sim {

struct BLEDeviceInfo {
    std::string address;
    std::string name;
    bool isConnected;
    int rssi = 0;              // Signal strength in dBm
    void* peripheral = nullptr; // Platform-specific peripheral handle
};

} // namespace vehicle_sim

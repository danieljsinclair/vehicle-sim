#include "vehicle-sim/ble/BLEManagerBase.h"
#include "vehicle-sim/boundary/OBD2Protocol.h"
#include "vehicle-sim/boundary/ELM327Transport.h"

#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

namespace vehicle_sim {

// ================================================
// OBD2 UUIDs - Already defined in header as constexpr
// ================================================

// ================================================
// OBD2 PIDs - Already defined in header as constexpr
// ================================================

// ================================================
// BLEManagerBase Implementation
// ================================================

BLEManagerBase::BLEManagerBase()
    : connected_(false) {
}

void BLEManagerBase::setDeviceFoundCallback(DeviceCallback callback) {
    device_callback_ = std::move(callback);
}

void BLEManagerBase::setDataReceivedCallback(DataCallback callback) {
    data_callback_ = std::move(callback);
}

void BLEManagerBase::setConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = std::move(callback);
}

// ================================================
// OBD2 Command Building
// ================================================

OBD2Response BLEManagerBase::queryPID(uint8_t pid) {
    // Use ELM327Transport to build ASCII command
    std::string cmd = boundary::ELM327Transport::buildOBD2Query(OBD2_MODE_LIVE_DATA, pid);
    // Convert ASCII string to bytes for BLE send
    std::vector<uint8_t> bytes(cmd.begin(), cmd.end());
    send(bytes);

    // The actual response will come via the data callback
    return OBD2Response{};  // Return empty - response comes async
}

std::vector<uint8_t> BLEManagerBase::parseASCIIResponseToBinary(const std::vector<uint8_t>& asciiData) {
    // Convert bytes to string (ELM327 sends ASCII text)
    std::string response(asciiData.begin(), asciiData.end());

    // Use ELM327Transport to parse ASCII hex to binary
    auto binaryData = boundary::ELM327Transport::parseOBD2Response(response);

    if (binaryData) {
        return *binaryData;
    }

    // Not a valid OBD2 response - could be prompt, echo, or error
    return {};
}

bool BLEManagerBase::initializeELM327() {
    std::cout << "[BLEManagerBase] Initializing ELM327 adapter..." << std::endl;

    // Set up OBD2Protocol to send ASCII commands via this BLEManagerBase
    obd2_protocol_.setSendCallback([this](const std::string& asciiCommand) {
        // Convert ASCII command string to bytes for BLE send
        std::vector<uint8_t> bytes(asciiCommand.begin(), asciiCommand.end());
        send(bytes);
    });

    // Send initialization sequence
    obd2_protocol_.initialize();

    std::cout << "[BLEManagerBase] ELM327 initialization commands sent" << std::endl;
    return true;
}

std::optional<domain::VehicleDetectionResult> BLEManagerBase::initializeOBD2WithDetection() {
    std::cout << "[BLEManagerBase] Initializing ELM327 with auto-detection..." << std::endl;

    // Set up OBD2Protocol to send ASCII commands via this BLEManagerBase
    obd2_protocol_.setSendCallback([this](const std::string& asciiCommand) {
        // Convert ASCII command string to bytes for BLE send
        std::vector<uint8_t> bytes(asciiCommand.begin(), asciiCommand.end());
        send(bytes);
    });

    // Send initialization sequence
    obd2_protocol_.initialize();

    // Note: In a real async implementation, we would:
    // 1. Send VIN query
    // 2. Wait for and parse multi-frame response
    // 3. Send fuel type query
    // 4. Parse fuel type response
    // 5. Return detection result
    //
    // For now, this method sets up the protocol and returns an empty result.
    // The caller must call processOBD2Data() with incoming responses.

    std::cout << "[BLEManagerBase] Auto-detection ready - send queries and process responses" << std::endl;
    return obd2_protocol_.detectVehicle();
}

void BLEManagerBase::processOBD2Data(const std::string& asciiData) {
    obd2_protocol_.processIncomingData(asciiData);
}

std::string BLEManagerBase::signalQuality(int rssi) {
    if (rssi >= RSSI_EXCELLENT) return "Excellent";
    if (rssi >= RSSI_GOOD) return "Good";
    if (rssi >= RSSI_FAIR) return "Fair";
    return "Poor";
}

// ================================================
// OBD2 Polling
// ================================================

void BLEManagerBase::startOBD2Polling(int interval_ms) {
    if (polling_active_) {
        return;  // Already polling
    }

    polling_interval_ms_ = interval_ms;
    polling_active_ = true;

    polling_thread_ = std::thread([this]() {
        std::cout << "[BLEManagerBase] Starting OBD2 polling at "
                  << polling_interval_ms_ << "ms intervals" << std::endl;

        // Wait a moment for characteristic notifications to be set up
        std::this_thread::sleep_for(std::chrono::milliseconds(POST_CONNECT_SETUP_DELAY_MS));

        while (polling_active_ && connected_) {
            // Query each PID sequentially
            // Throttle position
            queryPID(OBD2PIDs::THROTTLE_POSITION);
            std::this_thread::sleep_for(std::chrono::milliseconds(PID_QUERY_DELAY_MS));

            // Vehicle speed
            queryPID(OBD2PIDs::VEHICLE_SPEED);
            std::this_thread::sleep_for(std::chrono::milliseconds(PID_QUERY_DELAY_MS));

            // Engine RPM
            queryPID(OBD2PIDs::ENGINE_RPM);
            std::this_thread::sleep_for(std::chrono::milliseconds(PID_QUERY_DELAY_MS));

            // Engine load
            queryPID(OBD2PIDs::ENGINE_LOAD);
            std::this_thread::sleep_for(std::chrono::milliseconds(PID_QUERY_DELAY_MS));

            // Coolant temp
            queryPID(OBD2PIDs::COOLANT_TEMP);

            // Wait remaining interval time
            std::this_thread::sleep_for(std::chrono::milliseconds(
                polling_interval_ms_ - TOTAL_PID_QUERY_TIME_MS > 0 ? polling_interval_ms_ - TOTAL_PID_QUERY_TIME_MS : PID_QUERY_DELAY_MS
            ));
        }

        std::cout << "[BLEManagerBase] OBD2 polling stopped" << std::endl;
    });
}

void BLEManagerBase::stopOBD2Polling() {
    polling_active_ = false;
    if (polling_thread_.joinable()) {
        polling_thread_.join();
    }
}

bool BLEManagerBase::initializeCANMonitor() {
    std::cout << "[BLEManagerBase] Initializing ELM327 for CAN monitor mode..." << std::endl;

    auto commands = boundary::ELM327Transport::buildCANMonitorInitSequence();
    for (const auto& cmd : commands) {
        std::vector<uint8_t> bytes(cmd.command.begin(), cmd.command.end());
        send(bytes);
        std::this_thread::sleep_for(std::chrono::milliseconds(cmd.delayMs));
    }

    can_mode_ = true;
    std::cout << "[BLEManagerBase] CAN monitor mode initialized" << std::endl;
    return true;
}

void BLEManagerBase::startCANMonitor(int interval_ms) {
    // CAN monitor mode doesn't need a polling thread.
    // ELM327 streams CAN frames continuously after ATMA command.
    // Data arrives via BLE notifications → invokeDataCallback() → CAN frame parsing.
    can_mode_ = true;
    std::cout << "[BLEManagerBase] CAN monitor mode active - receiving frames via BLE notifications" << std::endl;
}

void BLEManagerBase::stopCANMonitor() {
    can_mode_ = false;
    // Send stop monitoring command
    std::string stopCmd = "ATMA\r";
    std::vector<uint8_t> bytes(stopCmd.begin(), stopCmd.end());
    send(bytes);
    std::cout << "[BLEManagerBase] CAN monitor mode stopped" << std::endl;
}

// ================================================
// Device Management (Common Implementation)
// ================================================

void BLEManagerBase::addDiscoveredDevice(const BLEDeviceInfo& device) {
    std::lock_guard<std::mutex> lock(devices_mutex_);

    // Check for duplicates
    for (const auto& existing : discovered_devices_) {
        if (existing.address == device.address) {
            return;  // Already have this device
        }
    }

    discovered_devices_.push_back(device);
    std::cout << "[BLEManagerBase] Added device: " << device.name
              << " (RSSI: " << device.rssi << ")" << std::endl;

    // Invoke callback
    invokeDeviceCallback(device);
}

void BLEManagerBase::clearDiscoveredDevices() {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    discovered_devices_.clear();
}

std::optional<BLEDeviceInfo> BLEManagerBase::findDeviceByAddress(const std::string& address) const {
    std::lock_guard<std::mutex> lock(devices_mutex_);

    for (const auto& device : discovered_devices_) {
        if (device.address == address) {
            return device;
        }
    }

    return std::nullopt;
}

// ================================================
// Callback Invocation
// ================================================

void BLEManagerBase::invokeDeviceCallback(const BLEDeviceInfo& device) {
    if (device_callback_) {
        device_callback_(device);
    }
}

void BLEManagerBase::invokeDataCallback(const std::vector<uint8_t>& data) {
    if (!data_callback_) {
        return;
    }

    if (can_mode_) {
        // CAN monitor mode: parse CAN frame from ASCII
        std::string asciiStr(data.begin(), data.end());
        auto frame = boundary::ELM327Transport::parseCANFrame(asciiStr);
        if (frame && frame->data.size() == 8) {
            // Convert CANFrame to DBC translator format:
            // [canId_lo, canId_hi, data_byte_0, ..., data_byte_7]
            std::vector<uint8_t> binary(10);
            binary[0] = frame->canId & 0xFF;
            binary[1] = (frame->canId >> 8) & 0xFF;
            std::copy(frame->data.begin(), frame->data.end(), binary.begin() + 2);
            data_callback_(binary);
        }
        return;
    }

    // OBD2 mode (existing): parse ASCII hex to binary
    std::vector<uint8_t> binaryData = parseASCIIResponseToBinary(data);
    if (!binaryData.empty()) {
        data_callback_(binaryData);
    }
}

void BLEManagerBase::invokeConnectionCallback(bool connected, const std::string& device_id) {
    if (connection_callback_) {
        connection_callback_(connected, device_id);
    }
}

void BLEManagerBase::setConnectionState(bool connected, const std::string& device_id) {
    connected_ = connected;
    connected_device_id_ = device_id;
    invokeConnectionCallback(connected, device_id);
}

} // namespace vehicle_sim
#include "vehicle-sim/cli/CliOptions.h"
#include "vehicle-sim/domain/VehicleConfig.h"

#include <iostream>

namespace vehicle_sim::cli {

CliOptions parseArgs(int argc, char* argv[]) {
    CliOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            opts.help_requested = true;
        } else if (arg == "--scan" || arg == "-s") {
            opts.scan_mode = true;
        } else if (arg == "--connect" || arg == "-c") {
            if (i + 1 < argc) {
                opts.connect_mode = true;
                opts.connect_address = argv[++i];
            } else {
                opts.error_message = "Error: --connect requires an address argument";
            }
        } else if (arg == "--vehicle" || arg == "-v") {
            if (i + 1 < argc) {
                opts.vehicle_type = argv[++i];
            } else {
                opts.error_message = "Error: --vehicle requires a type argument";
            }
        } else if (arg == "--list" || arg == "-l") {
            opts.list_signals = true;
        } else if (arg == "--format" || arg == "-f") {
            if (i + 1 < argc) {
                opts.format = argv[++i];
            } else {
                opts.error_message = "Error: --format requires a format argument";
            }
        } else if (arg == "--interval" || arg == "-i") {
            if (i + 1 < argc) {
                opts.update_interval_ms = std::stoi(argv[++i]);
            } else {
                opts.error_message = "Error: --interval requires a number argument";
            }
        } else if (arg == "--simulate" || arg == "-m") {
            opts.simulate_mode = true;
        } else {
            opts.error_message = "Error: Unknown argument: " + arg;
        }
    }

    return opts;
}

void printHelp(std::ostream& out, const domain::VehicleConfigRegistry& registry) {
    out << "vehicle-sim - Vehicle OBD2 Telemetry Display\n\n";
    out << "USAGE:\n";
    out << "  vehicle-sim [OPTIONS]\n\n";
    out << "OPTIONS:\n";
    out << "  --scan              Scan for BLE OBD2 adapters\n";
    out << "  --connect <addr>    Connect to specific BLE adapter address\n";
    out << "  --vehicle <type>    Force vehicle type (auto-detected if omitted)\n";
    out << "  --list              List supported signals for each vehicle\n";
    out << "  --format <fmt>      Output format: json, csv, or plain (default: plain)\n";
    out << "  --interval <ms>     Update interval in milliseconds (default: 500)\n";
    out << "  --simulate          Demo mode with mock telemetry (no hardware needed)\n";
    out << "  --help              Show this help message\n\n";

    auto vehicles = registry.getRegisteredVehicles();
    if (!vehicles.empty()) {
        out << "SUPPORTED VEHICLES (auto-detected on connect):\n";
        for (const auto& id : vehicles) {
            const auto* cfg = registry.getConfig(id);
            if (cfg) {
                out << "  " << id << "  (" << cfg->vehicleName << ")\n";
            }
        }
        out << "\n";
    }

    out << "EXAMPLES:\n";
    out << "  vehicle-sim --simulate\n";
    out << "  vehicle-sim --scan\n";
    out << "  vehicle-sim --connect <addr>                         # auto-detect\n";
    out << "  vehicle-sim --connect <addr> --vehicle tesla_model3  # force Tesla\n\n";
    out << "REQUIREMENTS:\n";
    out << "  For real data: Connect a BLE OBD2 adapter to your vehicle's OBD-II port.\n";
    out << "  Vehicle type is auto-detected via VIN query. Use --vehicle to override.\n";
}

void printSupportedSignals(std::ostream& out, const domain::VehicleConfigRegistry& registry) {
    auto vehicles = registry.getRegisteredVehicles();
    for (const auto& id : vehicles) {
        const auto* cfg = registry.getConfig(id);
        if (!cfg) continue;

        out << "\n" << cfg->vehicleName << " (" << id << "):\n";
        for (const auto& [signalName, fieldName] : cfg->signalMappings) {
            out << "  " << signalName << " -> " << fieldName << "\n";
        }
        out << "  Protocol: " << (cfg->isCANProtocol ? "CAN (DBC)" : "OBD2 (SAE J1979)") << "\n";
    }
    out << "\n";
}

} // namespace vehicle_sim::cli

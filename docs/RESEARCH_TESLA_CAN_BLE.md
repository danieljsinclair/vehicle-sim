# Tesla CAN and BLE Integration Research

## Executive Summary
Tesla vehicles use raw CAN bus communication, not standard OBD-II protocol. This document captures research findings for future implementation of Tesla signal parsing and BLE integration.

## Key Findings

### Tesla Communication Protocol
- **Protocol**: Raw CAN bus (not standard OBD-II request/response)
- **Access Point**: OBD-II diagnostic port (Model 3/Y - only powered when vehicle is on)
- **Message Format**: CAN frames with specific IDs defined in DBC files

### Tesla Model 3 CAN Resources

#### Open Source DBC Files
1. **[Adminius/tesla-can-dbc](https://github.com/Adminius/tesla-can-dbc)**
   - DBC file for Tesla Model 3
   - Used by Onyx M2 Project for myTeslaModel3
   - Updated over years via Tesla Owners Online forum research

2. **[joshwardell/model3dbc](https://github.com/joshwardell/model3dbc)**
   - Widely used Tesla Model 3 CAN DBC file
   - Maintained by Josh Wardell (CAN bus researcher)
   - Popular reverse-engineering work

#### CAN Bus Analysis Tools
- **tes•LAX ([teslax.app](https://teslax.app))**
  - iOS app with known Tesla signal database
  - Supports Model 3, Model S/X, and Rivian
  - Works with ScanTool OBDLink MX+ (Bluetooth Classic)
  - 11-bit message identifier analysis

### iOS BLE OBD-II Libraries

#### Swift Libraries
1. **[kkonteh97/SwiftOBD2](https://github.com/kkonteh97/SwiftOBD2)**
   - Swift package for ELM327 OBD2 adapters
   - Real-time vehicle data retrieval
   - Sample app included
   - [Medium Guide](https://medium.com/@kkonteh97/build-an-obd2-car-diagnostics-app-with-swift-02e91b6f828e)

2. **[lemberg/obd2-swift-lib](https://github.com/lemberg/obd2-swift-lib)**
   - Car onboard diagnostics Swift library
   - Generic OBD2 implementation

### Hardware Considerations
- **ELM327 Adapter**: Common OBD-II to Bluetooth bridge
- **Bluetooth Classic**: Required for some tools (OBDLink MX+)
- **BLE (Bluetooth Low Energy)**: iOS native, but may need specific adapter
- **Tesla Model 3/Y**: OBD-II port only powered when vehicle is on (post-2021)

### Signal Types Available (from DBC research)
- Vehicle speed
- Battery state of charge
- Gear position
- RPM
- Torque
- Throttle position
- Various ECU status messages

## Technical Architecture Recommendations

### Approach Options

#### Option 1: DBC-Based Parsing
- **Pros**: Standard approach, extensible, well-documented
- **Cons**: Requires DBC parser library
- **Best for**: Full-featured implementation

#### Option 2: Hardcoded Signal Registry
- **Pros**: Simple, no external dependencies
- **Cons**: Less flexible, manual signal management
- **Best for**: MVP with limited signals

### Recommended Architecture Layers
```
┌─────────────────────────────────────┐
│  iOS UI (Swift/SwiftUI)            │
│  - Live data display                │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Signal Bridge (Swift-C++)          │
│  - Domain value conversion           │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Tesla Signal Parser (C++)           │
│  - CAN frame interpretation          │
│  - Signal extraction                 │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  BLE Transport (C++/Swift)          │
│  - ELM327 communication             │
│  - CAN frame reception              │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Hardware                          │
│  - Tesla Model 3 OBD-II port       │
│  - ELM327 BLE adapter               │
└─────────────────────────────────────┘
```

## Implementation Notes

### For MVP v1
1. **Choose ONE signal**: Speed or Battery State of Charge (simple, high value)
2. **Use existing DBC**: Don't reverse-engineer from scratch
3. **Start with hardcoded**: Simpler for initial proof of concept
4. **Mock BLE initially**: Validate parser before hardware integration

### SOLID Considerations
- **Transport Abstraction**: Interface for BLE vs other transport
- **Parser Strategy**: Extensible for different vehicle DBC files
- **DI for Testing**: All dependencies injectable
- **Signal Registry**: Open for extension (add signals without modifying parser)

## References
- [Tesla Owners Online - Diagnostic Port](https://www.teslaownersonline.com/threads/diagnostic-port-and-data-access.7502/page-104)
- [Open Vehicles - DBC Based Vehicles](https://docs.openvehicles.com/en/latest/components/vehicle_dbc/docs/index.html)
- [TeslaTap - Internal Vehicle Data](https://teslatap.com/modifications/extracting-internal-vehicle-data/)
- [CS Selectronics - Tesla Data Dashboard](https://www.csselectronics.com/pages/tesla-data-dashboard-telematics-can-bus-grafana)

## Date
2026-04-05

## Status
Parked for future implementation. Focus: iOS C++ build integration.

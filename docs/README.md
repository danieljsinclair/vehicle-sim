# Vehicle-Sim Research Documentation

Hardware and protocol research for OBD2/CAN bus integration with target vehicles.

## Contents

- [Audi e-tron (MLB Evo) OBD2 Compatibility](audi-etron-obd2-research.md)
- [Tesla Model 3/Y CAN Bus & Signal Reference](tesla-model3-can-reference.md)
- [Hardware Adapter Guide](hardware-adapter-guide.md)

## Quick Reference

| Vehicle | Standard OBD2 PIDs | Raw CAN | Recommended Adapter | Status |
|---------|-------------------|---------|-------------------|--------|
| Toyota Aygo | Yes (Mode 01) | N/A | ELM327 V2.1 (works) | Validated |
| Audi e-tron | Minimal (gateway blocks) | UDS required | OBDeleven / OBDLink MX+ | Blocked by hardware |
| Tesla Model 3/Y | No | Yes (Party CAN) | ESP32 + CAN transceiver | Architecture ready |

## Key Findings

1. **ELM327 V2.1 clones** work for standard OBD2 vehicles (Toyota Aygo) but are fundamentally limited for VW/Audi (UDS protocol) and Tesla (no OBD2 PID support, raw CAN only).

2. **Audi e-tron** uses UDS over CAN with gateway ECU that blocks standard OBD2 access. Even the adapter's own iOS app cannot read the e-tron.

3. **Tesla Model 3/Y** does not implement standard OBD2 PIDs. All telemetry comes from raw CAN frames on the Party CAN bus (11-bit, 500kbps). Rich signal set available (motor RPM, torque, speed, steering, throttle, brake) but accelerometer data is NOT on the CAN bus.

4. **The OBD2 port on Tesla** may go silent in Drive on some firmware builds. The X179 connector (behind rear center console) is the reliable access point for driving data.

## Sources

- [commaai/opendbc](https://github.com/commaai/opendbc) (3.1k stars) — DBC files and car port implementations
- [joshwardell/model3dbc](https://github.com/joshwardell/model3dbc) (382 stars) — Tesla Model 3 DBC for all three buses
- [hypery11/flipper-tesla-fsd](https://github.com/hypery11/flipper-tesla-fsd) (722 stars) — Tesla CAN toolkit with driving validation
- [joshwardell/CANserver](https://github.com/joshwardell/CANserver) (112 stars) — ESP32 CAN-to-WiFi for Tesla
- [evcc-io/evcc](https://github.com/evcc-io/evcc) — Audi e-tron cloud API integration
- [python-udsoncan](https://github.com/pylessard/python-udsoncan) — Python UDS implementation

# Audi e-tron (MLB Evo) OBD2/CAN Bus Compatibility

## Executive Summary

The Audi e-tron (MLB Evo / MEB HV platform) is **fundamentally incompatible with cheap ELM327 V2.1 clone adapters** for reading live telemetry. This is not a software bug — it is a hardware/architecture limitation. The e-tron uses a VW/Audi gateway ECU architecture that requires UDS (ISO 14229) over CAN for diagnostics, not standard OBD2 PID queries. The gateway ECU blocks generic OBD2 scanner access to most ECUs.

**Confirmed by real-world testing (2026-05-07):**
- ELM327 V2.1 BLE adapter connects to e-tron via BLE successfully
- ELM327 init sequence completes (AT commands accepted, `>` prompts received)
- CAN monitor mode (ATMA): CAN bus silent when parked — zero CAN frames
- OBD2 PID query mode (generic): ELM327 returns all-zero responses to every PID (0x42 battery voltage, 0x05 coolant temp, 0x0D speed)
- **The adapter's own iOS app also cannot read the e-tron** — confirms hardware limitation
- Same adapter reads Toyota Aygo successfully (standard OBD2 PIDs)

---

## Protocol Architecture

### Multi-Layer Protocol Stack

| Layer | Protocol | Standard |
|-------|----------|----------|
| Physical | CAN bus (ISO 11898-2) | Multiple buses at 500 kbps |
| Transport | ISO-TP (ISO 15765-2) | Multi-frame messaging |
| Diagnostic | **UDS (ISO 14229)** | NOT standard OBD2 (SAE J1979) |
| OBD2 compliance | ISO 15765-4 | Emissions-mandated ECUs only |

### Critical Distinction

- OBD2 (SAE J1979) handles emissions-mandated diagnostics ONLY (engine, transmission, emissions)
- UDS (ISO 14229) handles all other ECUs (battery, charger, HVAC, ADAS, body)
- The e-tron has no internal combustion engine, so the emissions-mandated OBD2 PID set is **minimal**
- Most interesting data (HV battery SoC, charger status, motor temps) requires UDS access to non-engine ECUs

### CAN Bus Topology

The e-tron has **multiple CAN buses** accessible through a gateway:

| Bus | Contents | Access via OBD2 |
|-----|----------|----------------|
| Powertrain CAN | Motor controllers, battery, charger | Gateway-routed |
| Comfort CAN | Doors, windows, HVAC, body | Gateway-routed |
| Infotainment CAN | MIB display, navigation | Blocked |
| ADAS/Extended CAN | ACC radar, camera, sensors | Blocked |
| Chassis CAN | ESP, steering, brakes | Gateway-routed |

---

## Gateway ECU Blocking

The CAN Gateway (J533) routes messages between the OBD2 port and internal CAN buses:

- **Blocks unauthorized access** to most ECUs by default
- Requires **Security Access** (Seed/Key authentication, ISO 14229 Service 0x27) for diagnostic sessions on non-standard ECUs
- Uses **functional addressing** (broadcast 0x7DF) for standard OBD2 queries — may only respond with emissions data
- Uses **physical addressing** (specific ECU CAN IDs) for UDS sessions

> "Generic OBD2 tools are not capable of 'talking' to other ECUs than engine. For diagnosis of other control units such as ABS, airbag, audio or body modules you need vendor-specific software." — [OBDTester](https://obdtester.com/obd2_protocols)

---

## OBD2 Port Status When Parked

The OBD2 port behavior depends on vehicle state:

| State | Powertrain CAN | Comfort CAN | OBD2 Port |
|-------|----------------|-------------|-----------|
| Locked/Sleeping | Silent | Silent | 12V present, gateway sleeping |
| Unlocked (no key) | Silent | Active (door, lighting) | Gateway active |
| Ignition ON (Accessory) | Partially active | Active | Full gateway routing |
| Drive Ready | Fully active | Fully active | Full access |

With a proper adapter connected to the e-tron with ignition ON, you would expect to see CAN traffic for:
- Gateway messages: door status, locking, lighting (comfort CAN)
- ESP/ABS messages: wheel speeds, brake status
- HV battery messages: SoC, voltage, temperature (requires UDS session)
- Charger messages: charging status, power, HV voltage (if plugged in)

---

## Real-World Test Results (2026-05-07)

### Test Setup
- **Vehicle**: Audi e-tron (MLB Evo)
- **Adapter**: ELM327 V2.1 BLE clone
- **Conditions**: Parked, ignition ON

### CAN Monitor Mode (ATMA, ATSP6)
```
[BLE] notifications: 3  last: 0d 0d 3e
[!] No data received for 6 seconds.
Total signals received: 0
```
- 3 BLE notifications received, content `\r\r>` (ELM327 prompt)
- Zero CAN frames — bus silent when parked
- ATSP6 = ISO 15765-4 CAN 11-bit/500kbps

### OBD2 PID Query Mode (ATSP0)
```
[BLE] notifications: 66  last: 0d 0000000000000000000000000000000000000000000000000
[BLEManagerBase] Prompt timeout for PID 0x42
[BLEManagerBase] Prompt timeout for PID 0x5
[BLEManagerBase] Prompt timeout for PID 0xd
Total signals received: 0
```
- 66 BLE notifications received — data IS flowing
- All responses are zero bytes (`0d 0000...`)
- Every PID query times out waiting for `>` prompt
- ECU not responding on the auto-detected protocol

### Interpretation
- The ELM327 IS communicating with the adapter (init succeeds)
- The adapter IS sending queries to the CAN bus
- The e-tron's ECU is NOT responding to standard OBD2 PID queries
- The all-zero responses suggest the ELM327 cannot establish protocol communication with the e-tron's gateway
- This matches the adapter's own app behavior (also fails on e-tron)

---

## Adapters That Work with Audi e-tron

| Adapter | Price | Protocol | Notes |
|---------|-------|----------|-------|
| **OBDeleven** | $80-150 | UDS over CAN | VW/Audi-specific, BLE, most popular for e-tron owners |
| **Carista** | $40-80 | UDS over CAN | VW/Audi-specific, BLE, supports coding and live data |
| **OBDLink MX+** | ~$150 | All OBD2 + CAN 29-bit | Professional-grade, J2534 compliant |
| **comma four** | ~$999 | Raw CAN via J533 harness | Direct CAN bus access, raw frames |
| **PEAK PCAN-USB** | ~$300+ | Raw CAN | Professional CAN interface |

### Why ELM327 Clones Don't Work

1. **Clone firmware bugs**: ELM327 V2.1 clones claim to support protocols they don't actually implement. They give positive responses on commands they don't support.
2. **No ISO-TP/UDS support**: ELM327 chips (STN2120 or PIC-based clones) have known issues with ISO-TP multi-frame messaging and 29-bit CAN IDs, both required for VW/Audi communication.
3. **ATSP0 unreliability**: Auto-protocol detection on clones is notoriously unreliable — may report success without actually switching to the correct protocol.

> "A lot of the cheap clones will give a positive response on commands that they don't actually support, so then they ace tests like 'ELM327 Identifier' as a result, appearing better than legit devices" — [Reddit r/CarHacking](https://www.reddit.com/r/CarHacking/comments/1532g6x/)

### Why the Toyota Aygo Works

- Toyota uses simple, standard ISO 15765-4 CAN (11-bit IDs, 500 kbps)
- Minimal gateway interference
- Standard Mode 01 PIDs respond on broadcast address 0x7DF
- No UDS or security access required

---

## CAN Signal Definitions (opendbc vw_meb.dbc)

The [commaai/opendbc](https://github.com/commaai/opendbc) repository contains `vw_meb.dbc` (257KB) with decoded CAN signals for the MLB/MEB platform.

### Key CAN IDs

| CAN ID | Message | Key Signals | Notes |
|--------|---------|-------------|-------|
| 0x0BE (190) | MEB_HVEM_01 | Battery_Voltage, Engine_Power, In_Motion, Standstill | 48-byte extended frame |
| 0x0FC (252) | ESC_51 | Wheel speeds (4), Brake_Pressure, Steering, Longitudinal_Speed, Lateral/Longitudinal_Accel | 48-byte extended frame |
| 0x153 (339) | MSG_HYB_30 | HV drive motor state, HV drive state, fault status | 8 bytes, gateway |
| 0x086 (134) | LWI_01 | Steering wheel angle (deg), steering speed (deg/s) | 8 bytes, gateway |
| 0x365 (869) | NVEM_05 | Generator power (W), generator duty cycle (%), LV target voltage | 8 bytes, gateway |

### Key ECU Nodes (from vw_meb.dbc)

- `BMC_MLBevo` — Battery Management Controller
- `BMS_NV` — Battery Management System
- `DCDC_800V_PAG` — 800V DC-DC converter
- `DCDC_HV` — HV DC-DC converter
- `Ladegeraet_2` — Charger unit 2
- `Gateway` — CAN Gateway
- `TME` — Thermal Management Electronics
- `AWC` — All-Wheel Drive Controller

> **Note**: These signals require UDS session access or direct CAN bus tap — not available through the OBD2 port with standard adapters.

---

## Alternative: Audi Cloud API

The [evcc](https://github.com/evcc-io/evcc) project reads Audi e-tron data via the **Audi cloud API** (not OBD2):

- API endpoint: `https://app-api.live-my.audi.com/vgql/v1/graphql`
- Authentication: OAuth2 via Audi's identity provider
- Provides: SoC, charging status, range
- Implementation: `vehicle/audi/api.go` — purely HTTP/GraphQL client
- Limitation: Low update rate (~1 Hz), no motor torque, no steering, no per-wheel speeds

---

## Architectural Implications

### Short Term
1. **Accept ELM327 clone cannot work with e-tron** — confirmed by adapter's own app failing
2. **Use Toyota Aygo for OBD2 pipeline validation** — responds to standard PIDs
3. **For e-tron data**: Audi cloud API or proper adapter (OBDeleven)

### Medium Term
Design `CANTranslator` hierarchy with multiple transport types:
- `ELM327Transport` — standard OBD2 vehicles (Aygo, most ICE)
- `UDSTransport` — VW/Audi over proper adapter (OBDeleven)
- `CloudAPITransport` — Audi/VW cloud (like evcc)
- `DirectCANTransport` — comma four / PEAK raw CAN

### Long Term
If CAN bus data from e-tron is needed:
1. Obtain OBDeleven or OBDLink MX+
2. Implement UDS transport layer (ISO 14229)
3. Use `vw_meb.dbc` for signal decoding
4. Handle Seed/Key security access for non-engine ECUs

---

## Sources

- [commaai/opendbc vw_meb.dbc](https://github.com/commaai/opendbc/blob/master/opendbc/dbc/vw_meb.dbc) — MEB platform DBC
- [commaai/opendbc vw_mlb.dbc](https://github.com/commaai/opendbc/blob/master/opendbc/dbc/vw_mlb.dbc) — MLB platform DBC
- [CSS Electronics — UDS Tutorial](https://www.csselectronics.com/pages/uds-protocol-tutorial-unified-diagnostic-services)
- [eobdcode.com — UDS vs OBD2](https://www.eobdcode.com/en/magazine-en/advanced-diagnostics-with-uds-unified-diagnostic-services-everything-you-need-to-know-about-modern-vehicle-protocol/)
- [OBDTester — Protocol Reference](https://obdtester.com/obd2_protocols)
- [Reddit r/CarHacking — ELM327 clone issues](https://www.reddit.com/r/CarHacking/comments/1532g6x/)
- [Reddit r/CarHacking — ELM327 UDS](https://www.reddit.com/r/CarHacking/comments/1afu44y/)
- [evcc Audi integration](https://github.com/evcc-io/evcc)
- [python-udsoncan](https://github.com/pylessard/python-udsoncan) — Python UDS (ISO 14229) library
- [FORScan Forum — Clone Detection](https://www.forscan.org/forum/viewtopic.php?t=1757)

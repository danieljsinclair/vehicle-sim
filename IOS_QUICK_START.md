# iOS Build & Deploy - Quick Start

## Build and Deploy to iPhone (3 Steps)

```bash
# STEP 1: Build C++ core library
make ios

# STEP 2: Open Xcode project
open vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj

# STEP 3: In Xcode
# - Connect iPhone via USB
# - Select your device from dropdown
# - Set Team in Signing & Capabilities
# - Press Cmd+R to build and install
```

## Build and Run in Simulator (2 Steps)

```bash
# STEP 1: Build C++ core library
make ios

# STEP 2: Open Xcode project
open vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj

# In Xcode:
# - Select "iPhone Simulator" as target
# - Press Cmd+R to build and run
```

## Files

| File | Purpose |
|------|---------|
| `build-ios/Release/libvehicle-sim-core.a` | C++ static library |
| `vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj` | Xcode project |
| `vehicle-sim-ios/BUILD_DEPLOY_GUIDE.md` | Full deployment guide |

## Requirements

- macOS 14+
- Xcode 16+
- iOS 14.0+ (deployment target)
- Apple ID (free or paid developer)

## Troubleshooting

**Code signing error?**
- Open project in Xcode
- Go to Signing & Capabilities
- Set Team to your Apple ID

**Library linking error?**
- Run `make ios` to rebuild C++ library
- Clean Xcode build (Product → Clean Build Folder)

**App crashes on iPhone?**
- Trust the app: Settings → General → VPN & Device Management → Trust
- Check Console app for crash logs

For detailed instructions, see [vehicle-sim-ios/BUILD_DEPLOY_GUIDE.md](vehicle-sim-ios/BUILD_DEPLOY_GUIDE.md)

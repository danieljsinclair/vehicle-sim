# iOS Build and Deployment Guide

## Overview

The vehicle-sim iOS app consists of:
1. **C++ Core Library**: Cross-platform vehicle simulation logic
2. **Swift iOS App**: SwiftUI-based mobile app
3. **Objective-C++ Bridge**: Links C++ core to Swift UI

## Quick Start

### Step 1: Build C++ Core Library

From the project root:

```bash
make ios
```

This builds the C++ static library: `build-ios/Release/libvehicle-sim-core.a`

### Step 2: Build and Deploy iOS App

**Option A: Xcode IDE (Recommended)**

```bash
# Open the Xcode project
open vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj
```

In Xcode:
1. Select the **VehicleSimApp** scheme
2. Choose your target:
   - **iPhone Simulator** for testing without device
   - **Your iPhone** for physical deployment
3. Press **Cmd+R** to build and run

**Option B: Command Line Build**

```bash
# Build for iOS Simulator
xcodebuild -project vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj \
  -scheme VehicleSimApp \
  -configuration Release \
  -sdk iphonesimulator \
  -arch arm64

# Build for Physical Device
xcodebuild -project vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj \
  -scheme VehicleSimApp \
  -configuration Release \
  -sdk iphoneos \
  -arch arm64 \
  CODE_SIGN_IDENTITY="iPhone Developer"
```

## Physical Device Deployment

### Prerequisites
- Apple ID (free or paid developer account)
- iPhone connected to Mac via USB
- Xcode installed

### Steps

1. **Connect Your iPhone**
   - Use USB cable to connect iPhone to Mac
   - Trust this computer on your iPhone when prompted

2. **Configure Signing**
   - In Xcode, select your project → VehicleSimApp target
   - Go to **Signing & Capabilities** tab
   - Set **Team** to your Apple ID
   - For free Apple ID: "Automatically manage signing" works
   - For paid developer: Use your signing certificate

3. **Build and Install**
   - Select your iPhone from the device dropdown (top of Xcode)
   - Press **Cmd+R** or click the Run button
   - First time may take longer due to code signing

4. **Trust the App** (Required for first install)
   - On your iPhone: Settings → General → VPN & Device Management
   - Find your Apple ID under "Enterprise App"
   - Tap it and **Trust** the developer

## iOS Simulator Deployment

### Quick Install

```bash
# Build the app first
make ios

# Install to simulator
xcrun simctl install booted build-ios/Release-iphonesimulator/VehicleSimApp.app

# Launch the app
xcrun simctl launch booted com.axxiant.vehiclesim
```

### Open in Xcode Simulator

```bash
# Build with make ios
make ios

# Open simulator
open -a Simulator

# Find and install the .app bundle
open build-ios/Release-iphonesimulator/
```

## Troubleshooting

### Code Signing Issues

**Error**: "Code signing is required for product type 'Application'"

**Solution**:
1. Open project in Xcode
2. Go to Signing & Capabilities
3. Set Team to your Apple ID
4. For free accounts, enable "Automatically manage signing"

### Library Linking Issues

**Error**: "Undefined symbols for architecture arm64"

**Solution**:
```bash
# Rebuild C++ core library
make ios

# Clean Xcode build folder (Product → Clean Build Folder)
# Then rebuild in Xcode
```

### Simulator Not Booted

**Error**: "No booted simulators found"

**Solution**:
```bash
# Boot a simulator
xcrun simctl boot "iPhone 16 Pro"

# Or open Simulator app
open -a Simulator
```

### App Crashes on Launch

**Possible causes**:
1. Deployment target mismatch (requires iOS 14.0+)
2. Missing C++ library dependencies
3. Code signing trust issue

**Solution**:
```bash
# Check device logs
open -a Console.app
# Filter by "VehicleSim" to see crash logs
```

## Build Outputs

| Artifact | Location | Description |
|-----------|-----------|-------------|
| C++ Library | `build-ios/Release/libvehicle-sim-core.a` | Static C++ library |
| iOS App (Sim) | `build-ios/Release-iphonesimulator/VehicleSimApp.app` | Simulator app bundle |
| iOS App (Device) | `build-ios/Release-iphoneos/VehicleSimApp.app` | Device app bundle |

## CI/CD Integration

For automated builds, use:

```bash
#!/bin/bash
# ci-build-ios.sh

# Build C++ core
make ios

# Build iOS app
xcodebuild -project vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj \
  -scheme VehicleSimApp \
  -configuration Release \
  -sdk iphonesimulator \
  -arch arm64 \
  -derivedDataPath ./DerivedData \
  CODE_SIGNING_ALLOWED=NO
```

## Architecture

```
┌─────────────────────────────────────┐
│  SwiftUI iOS App (Swift)           │
│  - VehicleSimAppApp.swift          │
│  - ContentView.swift               │
│  - VehicleViewModel.swift          │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Objective-C++ Bridge (.mm)         │
│  - VehicleSimWrapper               │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  C++ Core Library (.a)             │
│  - VehicleSimulator                │
│  - PhysicsEngine                  │
│  - BLEManager                    │
│  - Domain objects                 │
└─────────────────────────────────────┘
```

## Dependencies

- **iOS**: 14.0+
- **Swift**: 5.0+
- **Xcode**: 16.0+
- **CMake**: 3.20+ (for C++ library build)

## Support

For issues:
1. Check Xcode Console for crash logs
2. Verify code signing configuration
3. Ensure C++ library is rebuilt after source changes
4. Check deployment target compatibility

## Date
2026-04-05

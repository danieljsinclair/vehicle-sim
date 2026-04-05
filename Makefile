# Makefile wrapper for vehicle-sim
# Handles both fresh clone and development builds
#
# IMPORTANT: Always use 'make' from project root, never run 'cmake' directly.
# Running 'cmake -S . -B .' will overwrite this Makefile and break the build.

BUILD_DIR ?= build
BUILD_TYPE ?= Release

# Default to parallel build using available CPU cores
MAKEFLAGS += -j$(shell sysctl -n hw.ncpu 2>/dev/null || echo 4)

.PHONY: all clean scrub test submodules check-cmake ios clean-ios ios-run ios-device

all: check-cmake submodules $(BUILD_DIR)/Makefile
	@cd $(BUILD_DIR) && $(MAKE)

# Check if running cmake directly (which would overwrite this Makefile)
check-cmake:
	@if [ -f CMakeCache.txt ] && ! grep -q "CMAKE_BUILD_TYPE:STRING=Release" CMakeCache.txt; then \
		echo "ERROR: Root CMakeCache.txt exists with wrong BUILD_TYPE."; \
		echo "This happens when running 'cmake -S . -B .' directly."; \
		echo "Please run 'make' instead."; \
		exit 1; \
	fi

# Initialize submodules (if any)
submodules:
	@if [ -f .gitmodules ]; then \
		echo "Initializing submodules..."; \
		git submodule update --init --recursive; \
	fi

$(BUILD_DIR)/Makefile: submodules
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ..

clean:
	@echo "Cleaning build artifacts..."
	@if [ -d $(BUILD_DIR) ]; then \
		$(MAKE) -C $(BUILD_DIR) clean 2>/dev/null || true; \
	fi
	@rm -rf $(BUILD_DIR)/CMakeCache.txt $(BUILD_DIR)/CMakeFiles 2>/dev/null || true

# Full clean - removes everything including build directory
scrub: clean
	@echo "Scrubbing all build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "Build artifacts scrubbed. Run 'make' to rebuild."

test: all
	@cd $(BUILD_DIR) && $(MAKE) test ARGS="-V --output-on-failure"

# iOS simulator build (builds C++ core library only)
ios: check-cmake submodules
	@echo "Building vehicle-sim C++ core library for iOS..."
	@rm -rf build-ios
	@mkdir -p build-ios
	@cd build-ios && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_IOS=ON -G Xcode ..
	@cd build-ios && xcodebuild -scheme vehicle-sim-core-ios -configuration $(BUILD_TYPE) -sdk iphonesimulator -arch arm64 build
	@echo ""
	@echo "iOS C++ core library built successfully: build-ios/Release/libvehicle-sim-core.a"
	@echo ""
	@echo "========================================"
	@echo "iOS APP BUILD & DEPLOYMENT:"
	@echo "========================================"
	@echo ""
	@echo "STEP 1: Open Xcode project:"
	@echo "   open vehicle-sim-ios/VehicleSim/VehicleSimApp.xcodeproj"
	@echo ""
	@echo "STEP 2: Build and run in Xcode:"
	@echo "   - Select 'VehicleSimApp' scheme"
	@echo "   - Choose target: iPhone Simulator OR your physical iPhone"
	@echo "   - Press Cmd+R to build and run"
	@echo ""
	@echo "STEP 3: For physical iPhone deployment:"
	@echo "   - Connect iPhone to Mac via USB"
	@echo "   - Select your device from target dropdown in Xcode"
	@echo "   - Go to Signing & Capabilities, set Team to your Apple ID"
	@echo "   - Press Cmd+R to build and install"
	@echo "   - First install: Settings → General → VPN & Device Management → Trust"
	@echo ""
	@echo "See vehicle-sim-ios/BUILD_DEPLOY_GUIDE.md for detailed instructions"
	@echo ""

clean-ios:
	@echo "Cleaning iOS build..."
	@rm -rf build-ios

cmake_minimum_required(VERSION 3.20)
project(vehicle-sim VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(Threads REQUIRED)

# Platform-specific settings
if(APPLE)
    # macOS specific frameworks if needed
elseif(LINUX)
    # Linux specific libraries if needed
endif()

# Add submodules if present
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/engine-sim-bridge/CMakeLists.txt)
    add_subdirectory(engine-sim-bridge ${CMAKE_BINARY_DIR}/engine-sim-bridge)
endif()

# Library target with core functionality (for testing and main executable)
add_library(vehicle-sim-lib
    src/physics/PhysicsEngine.cpp
    src/presentation/TelemetryFormatter.cpp
    src/ble/BLEManager.cpp
    src/config/ConfigLoader.cpp
)

target_link_libraries(vehicle-sim-lib
    PUBLIC
    Threads::Threads
)

target_include_directories(vehicle-sim-lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(vehicle-sim-lib PRIVATE -g)

# Main executable
add_executable(vehicle-sim
    src/main.cpp
)

target_link_libraries(vehicle-sim
    PRIVATE
    vehicle-sim-lib
)

target_compile_options(vehicle-sim PRIVATE -g)

# Set rpath for executable to find dylibs in build directory
if(APPLE)
    set_target_properties(vehicle-sim PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path"
        BUILD_RPATH "@executable_path"
    )
endif()

# Installation
install(TARGETS vehicle-sim
    RUNTIME DESTINATION bin
)

# Testing (optional)
option(BUILD_TESTS "Build vehicle-sim unit tests" ON)
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    # Add tests subdirectory if it exists
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt)
        add_subdirectory(test)
    endif()

    # Custom run_tests target
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --verbose
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running all tests"
    )
endif()

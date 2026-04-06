# Test CMake configuration
# NOTE: Physics engine tests are Phase 2 only (not included in Phase 0 build)
# Build with -DENABLE_PHASE_2_PHYSICS=ON for Phase 2 to include PhysicsEngine tests

add_executable(vehicle-sim-tests
    test_main.cpp
    test_example.cpp
    domain/VehicleSignal.test.cpp
    domain/TelemetrySignal.test.cpp
    domain/ISignalTranslator.test.cpp
    domain/TeslaSignalTranslator.test.cpp
    ble/BLEManager.test.cpp
    presentation/TelemetryFormatter.test.cpp
)

# Phase 2 physics test (only included when ENABLE_PHASE_2_PHYSICS=ON)
set(PHASE_2_TESTS
    physics/PhysicsEngine.test.cpp
)

# Append phase 2 tests if enabled
if(ENABLE_PHASE_2_PHYSICS)
    list(APPEND vehicle-sim-tests ${PHASE_2_TESTS})
endif()

target_link_libraries(vehicle-sim-tests
    PRIVATE
    GTest::gtest_main
    GTest::gmock
    vehicle-sim-lib
)

target_link_libraries(vehicle-sim-tests
    PRIVATE
    GTest::gtest_main
    GTest::gmock
    vehicle-sim-lib
)

target_include_directories(vehicle-sim-tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_test(NAME vehicle-sim-tests COMMAND vehicle-sim-tests)

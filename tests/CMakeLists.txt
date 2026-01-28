# SPDX-License-Identifier: MIT
find_package(Qt6 REQUIRED COMPONENTS Test)

# Define test data directory for use in tests
set(HWVIEW_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/testdata")
add_compile_definitions(HWVIEW_TEST_DATA_DIR="${HWVIEW_TEST_DATA_DIR}")

# Test subdirectories
add_subdirectory(common)
add_subdirectory(models)

# Note: The following tests are skipped because they require DeviceInfo from
# hwview_core which has platform-specific dependencies (hwview_udev on Linux,
# hwview_iokit on macOS, etc.) that require DeviceMonitor.
#
# TODO: Refactor the codebase to separate platform-independent functionality:
# 1. Move DeviceInfo and DeviceInfoPrivate to hwview_common
# 2. Have platform backends provide implementations of DeviceInfoPrivate
# 3. Build mock backend that doesn't pull in platform code
#
# add_subdirectory(${CMAKE_SOURCE_DIR}/src/backends/mock ${CMAKE_BINARY_DIR}/mock)
# add_subdirectory(mock)
# add_subdirectory(core)

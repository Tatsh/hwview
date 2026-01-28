# SPDX-License-Identifier: MIT

# Build common sources directly instead of linking to hwview_common
# to enable accurate coverage reporting
set(HWVIEW_COMMON_SOURCES
  ${CMAKE_SOURCE_DIR}/src/common/deviceinfo.cpp
  ${CMAKE_SOURCE_DIR}/src/common/importeddeviceinfo.cpp
  ${CMAKE_SOURCE_DIR}/src/common/namemappings.cpp)
set(HWVIEW_COMMON_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/common)

qt_add_executable(namemappingstest namemappingstest.cpp ${HWVIEW_COMMON_SOURCES})
target_include_directories(namemappingstest PRIVATE ${HWVIEW_COMMON_INCLUDE_DIRS})
target_link_libraries(namemappingstest PRIVATE Qt6::Core Qt6::Test)
target_compile_definitions(namemappingstest PRIVATE HWVIEW_TEST_DATA_DIR="${HWVIEW_TEST_DATA_DIR}")
add_test(NAME namemappingstest COMMAND namemappingstest)

qt_add_executable(deviceinfotest deviceinfotest.cpp ${HWVIEW_COMMON_SOURCES})
target_include_directories(deviceinfotest PRIVATE ${HWVIEW_COMMON_INCLUDE_DIRS})
target_link_libraries(deviceinfotest PRIVATE Qt6::Core Qt6::Test)
target_compile_definitions(deviceinfotest PRIVATE HWVIEW_TEST_DATA_DIR="${HWVIEW_TEST_DATA_DIR}")
add_test(NAME deviceinfotest COMMAND deviceinfotest)

qt_add_executable(importeddeviceinfotest importeddeviceinfotest.cpp ${HWVIEW_COMMON_SOURCES})
target_include_directories(importeddeviceinfotest PRIVATE ${HWVIEW_COMMON_INCLUDE_DIRS})
target_link_libraries(importeddeviceinfotest PRIVATE Qt6::Core Qt6::Test)
target_compile_definitions(importeddeviceinfotest PRIVATE HWVIEW_TEST_DATA_DIR="${HWVIEW_TEST_DATA_DIR}")
add_test(NAME importeddeviceinfotest COMMAND importeddeviceinfotest)

# SPDX-License-Identifier: MIT
qt_add_executable(namemappingstest namemappingstest.cpp)
target_link_libraries(namemappingstest PRIVATE hwview_common Qt6::Test)
target_compile_definitions(namemappingstest PRIVATE HWVIEW_TEST_DATA_DIR="${HWVIEW_TEST_DATA_DIR}")
add_test(NAME namemappingstest COMMAND namemappingstest)

qt_add_executable(deviceinfotest deviceinfotest.cpp)
target_link_libraries(deviceinfotest PRIVATE hwview_common Qt6::Test)
target_compile_definitions(deviceinfotest PRIVATE HWVIEW_TEST_DATA_DIR="${HWVIEW_TEST_DATA_DIR}")
add_test(NAME deviceinfotest COMMAND deviceinfotest)

qt_add_executable(importeddeviceinfotest importeddeviceinfotest.cpp)
target_link_libraries(importeddeviceinfotest PRIVATE hwview_common Qt6::Test)
target_compile_definitions(importeddeviceinfotest PRIVATE HWVIEW_TEST_DATA_DIR="${HWVIEW_TEST_DATA_DIR}")
add_test(NAME importeddeviceinfotest COMMAND importeddeviceinfotest)

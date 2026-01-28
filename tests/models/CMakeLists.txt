# SPDX-License-Identifier: MIT

# Build node.cpp directly instead of linking to hwview_models
# (hwview_models has many platform-specific dependencies)
qt_add_executable(nodetest nodetest.cpp ${CMAKE_SOURCE_DIR}/src/models/node.cpp)
target_include_directories(nodetest PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/models)
target_link_libraries(nodetest PRIVATE Qt6::Widgets Qt6::Test)
add_test(NAME nodetest COMMAND nodetest)

# Note: basetreemodeltest is skipped because hwview_models has dependencies
# on DeviceCache, DeviceInfo, and platform-specific systeminfo functions.
# TODO: Refactor hwview_models to separate Node and BaseTreeModel from
# the concrete model implementations that depend on DeviceCache.

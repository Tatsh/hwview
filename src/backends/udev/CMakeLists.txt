add_library(
  hwview_udev STATIC
  driverinfo.cpp
  systeminfo.cpp
  udevenumerate.cpp
  udevdeviceinfo.cpp
  udevmanager.cpp
  udevmonitor.cpp)

set_target_properties(hwview_udev PROPERTIES AUTOMOC ON)

target_include_directories(hwview_udev PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(hwview_udev PUBLIC hwview_common PkgConfig::UDEV Qt6::Core)

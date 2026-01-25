add_library(
  devmgmt_udev STATIC
  driverinfo.cpp
  systeminfo.cpp
  udevenumerate.cpp
  udevdeviceinfo.cpp
  udevmanager.cpp
  udevmonitor.cpp)

set_target_properties(devmgmt_udev PROPERTIES AUTOMOC ON)

target_include_directories(devmgmt_udev PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(devmgmt_udev PUBLIC PkgConfig::UDEV Qt6::Core)

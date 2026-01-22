add_library(
  devmgmt_udev STATIC
  udevenumerate.cpp
  udevmanager.cpp)

target_include_directories(devmgmt_udev PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(devmgmt_udev PUBLIC PkgConfig::UDEV Qt6::Core)

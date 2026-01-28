add_library(
  hwview_setupapi STATIC
  driverinfo.cpp
  setupapideviceinfo.cpp
  setupapimanager.cpp
  setupapimonitor.cpp
  systeminfo.cpp)

set_target_properties(hwview_setupapi PROPERTIES AUTOMOC ON)

target_include_directories(hwview_setupapi PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(hwview_setupapi PUBLIC
  hwview_common
  Qt6::Core
  setupapi
  cfgmgr32
  version
  wintrust
  crypt32)

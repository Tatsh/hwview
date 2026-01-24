add_library(
  devmgmt_setupapi STATIC
  setupapimanager.cpp
  setupapimonitor.cpp)

set_target_properties(devmgmt_setupapi PROPERTIES AUTOMOC ON)

target_include_directories(devmgmt_setupapi PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(devmgmt_setupapi PUBLIC
  Qt6::Core
  setupapi
  cfgmgr32)

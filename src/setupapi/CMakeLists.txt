add_library(
  devmgmt_setupapi STATIC
  setupapimanager.cpp)

target_include_directories(devmgmt_setupapi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(devmgmt_setupapi PUBLIC
  Qt6::Core
  setupapi
  cfgmgr32)

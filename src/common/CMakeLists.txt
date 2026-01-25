add_library(devmgmt_common STATIC namemappings.cpp importeddeviceinfo.cpp)

target_include_directories(
  devmgmt_common
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(devmgmt_common PUBLIC Qt6::Core)

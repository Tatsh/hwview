add_library(hwview_common STATIC namemappings.cpp importeddeviceinfo.cpp)

target_include_directories(
  hwview_common
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(hwview_common PUBLIC Qt6::Core)

add_library(
  hwview_iokit STATIC
  driverinfo.cpp
  iokitdeviceinfo.cpp
  iokitmanager.cpp
  iokitmonitor.cpp
  systeminfo.cpp)

set_target_properties(hwview_iokit PROPERTIES AUTOMOC ON)

target_include_directories(hwview_iokit PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(hwview_iokit PUBLIC
  Qt6::Core
  ${IOKIT_FRAMEWORK}
  ${COREFOUNDATION_FRAMEWORK}
  ${DISKARBITRATION_FRAMEWORK})

add_library(
  devmgmt_iokit STATIC
  iokitmanager.cpp)

target_include_directories(devmgmt_iokit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(devmgmt_iokit PUBLIC
  Qt6::Core
  ${IOKIT_FRAMEWORK}
  ${COREFOUNDATION_FRAMEWORK}
  ${DISKARBITRATION_FRAMEWORK})

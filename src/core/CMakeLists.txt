add_library(
  devmgmt_core STATIC
  deviceexport.cpp
  ../deviceinfo.cpp)

target_include_directories(
  devmgmt_core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(DEVMGMT_CLI_ONLY)
  target_compile_definitions(devmgmt_core PUBLIC DEVMGMT_CLI_ONLY)
  target_link_libraries(devmgmt_core PUBLIC devmgmt_common Qt6::Core)
else()
  target_link_libraries(devmgmt_core PUBLIC devmgmt_common Qt6::Core Qt6::Gui)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(devmgmt_core PUBLIC devmgmt_udev)
elseif(APPLE)
  target_link_libraries(devmgmt_core PUBLIC
    devmgmt_iokit
    ${IOKIT_FRAMEWORK}
    ${COREFOUNDATION_FRAMEWORK}
    ${DISKARBITRATION_FRAMEWORK})
elseif(WIN32)
  target_link_libraries(devmgmt_core PUBLIC
    devmgmt_setupapi
    setupapi
    cfgmgr32)
endif()

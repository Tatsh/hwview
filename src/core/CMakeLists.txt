add_library(
  hwview_core STATIC
  deviceexport.cpp
  ../deviceinfo.cpp)

target_include_directories(
  hwview_core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(HWVIEW_CLI_ONLY)
  target_compile_definitions(hwview_core PUBLIC HWVIEW_CLI_ONLY)
  target_link_libraries(hwview_core PUBLIC hwview_common Qt6::Core)
else()
  target_link_libraries(hwview_core PUBLIC hwview_common Qt6::Core Qt6::Gui)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(hwview_core PUBLIC hwview_udev)
elseif(APPLE)
  target_link_libraries(hwview_core PUBLIC
    hwview_iokit
    ${IOKIT_FRAMEWORK}
    ${COREFOUNDATION_FRAMEWORK}
    ${DISKARBITRATION_FRAMEWORK})
elseif(WIN32)
  target_link_libraries(hwview_core PUBLIC
    hwview_setupapi
    setupapi
    cfgmgr32)
endif()

add_subdirectory(backends)
add_subdirectory(models)

add_executable(
  devmgmt
  customizedialog.cpp
  devicecache.cpp
  deviceinfo.cpp
  devicemonitor.cpp
  driverdetailsdialog.cpp
  devmgmt.qrc
  main.cpp
  mainwindow.cpp
  namemappings.cpp
  propertiesdialog.cpp
  viewsettings.cpp)

target_include_directories(devmgmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  devmgmt
  PRIVATE devmgmt_models
          Qt6::Concurrent
          Qt6::Network
          Qt6::Widgets)

# Link platform-specific device enumeration library
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(devmgmt PRIVATE devmgmt_udev)
elseif(APPLE)
  target_link_libraries(devmgmt PRIVATE
    devmgmt_iokit
    ${IOKIT_FRAMEWORK}
    ${COREFOUNDATION_FRAMEWORK}
    ${DISKARBITRATION_FRAMEWORK})
elseif(WIN32)
  target_link_libraries(devmgmt PRIVATE
    devmgmt_setupapi
    setupapi
    cfgmgr32)
endif()

if(DEVMGMT_USE_KDE)
  target_compile_definitions(devmgmt PRIVATE DEVMGMT_USE_KDE)
  target_link_libraries(devmgmt PRIVATE KF6::ConfigWidgets KF6::CoreAddons KF6::I18n KF6::XmlGui)

  # Install KDE XML GUI resource file
  include(KDEInstallDirs)
  install(FILES devmgmtui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/devmgmt)
endif()

# Install data files
include(GNUInstallDirs)
install(
  FILES ${CMAKE_SOURCE_DIR}/data/name-mappings.json
  DESTINATION ${CMAKE_INSTALL_DATADIR}/devmgmt)

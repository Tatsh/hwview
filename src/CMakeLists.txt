add_subdirectory(models)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_subdirectory(udev)
endif()

add_executable(
  devmgmt
  customizedialog.cpp
  devicecache.cpp
  deviceinfo.cpp
  driverdetailsdialog.cpp
  devmgmt.qrc
  main.cpp
  mainwindow.cpp
  propertiesdialog.cpp
  viewsettings.cpp)

target_include_directories(devmgmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  devmgmt
  PRIVATE devmgmt_models
          Qt6::Concurrent
          Qt6::Network
          Qt6::Widgets)

# Link udev library only on Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(devmgmt PRIVATE devmgmt_udev)
endif()

if(DEVMGMT_USE_KDE)
  target_compile_definitions(devmgmt PRIVATE DEVMGMT_USE_KDE)
  target_link_libraries(devmgmt PRIVATE KF6::ConfigWidgets KF6::CoreAddons KF6::I18n KF6::XmlGui)

  # Install KDE XML GUI resource file
  include(KDEInstallDirs)
  install(FILES devmgmtui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/devmgmt)
endif()

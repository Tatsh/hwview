add_subdirectory(backends)
add_subdirectory(common)
add_subdirectory(core)

if(DEVMGMT_CLI_ONLY)
  add_executable(devmgmt main.cpp)
  target_compile_definitions(devmgmt PRIVATE DEVMGMT_HEADLESS)
  target_include_directories(devmgmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(devmgmt PRIVATE devmgmt_core Qt6::Core Qt6::Network)
  install(TARGETS devmgmt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
  add_subdirectory(models)
  add_executable(
    devmgmt
    customizedialog.cpp
    devicecache.cpp
    devicemonitor.cpp
    driverdetailsdialog.cpp
    devmgmt.qrc
    main.cpp
    mainwindow.cpp
    propertiesdialog.cpp
    viewsettings.cpp)
  target_include_directories(devmgmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(
    devmgmt
    PRIVATE devmgmt_common
            devmgmt_core
            devmgmt_models
            Qt6::Concurrent
            Qt6::Network
            Qt6::Widgets)

  if(DEVMGMT_USE_KDE)
    target_compile_definitions(devmgmt PRIVATE DEVMGMT_USE_KDE)
    target_link_libraries(devmgmt PRIVATE KF6::ConfigWidgets KF6::CoreAddons KF6::I18n KF6::XmlGui)
    include(KDEInstallDirs)
    install(FILES devmgmtui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/devmgmt)
  endif()
  install(TARGETS devmgmt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(
  FILES ${CMAKE_SOURCE_DIR}/data/name-mappings.json
  DESTINATION ${CMAKE_INSTALL_DATADIR}/devmgmt)

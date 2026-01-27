add_subdirectory(backends)
add_subdirectory(common)
add_subdirectory(core)

if(HWVIEW_CLI_ONLY)
  add_executable(hwview main.cpp)
  target_compile_definitions(hwview PRIVATE HWVIEW_HEADLESS)
  target_include_directories(hwview PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(hwview PRIVATE hwview_core Qt6::Core Qt6::Network)
  install(TARGETS hwview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
  add_subdirectory(models)
  add_executable(
    hwview
    customizedialog.cpp
    devicecache.cpp
    devicemonitor.cpp
    driverdetailsdialog.cpp
    hwview.qrc
    main.cpp
    mainwindow.cpp
    propertiesdialog.cpp
    viewsettings.cpp)
  target_include_directories(hwview PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(
    hwview
    PRIVATE hwview_common
            hwview_core
            hwview_models
            Qt6::Concurrent
            Qt6::Network
            Qt6::Widgets)

  if(HWVIEW_USE_KDE)
    target_compile_definitions(hwview PRIVATE HWVIEW_USE_KDE)
    target_link_libraries(hwview PRIVATE KF6::ConfigWidgets KF6::CoreAddons KF6::I18n KF6::XmlGui)
    include(KDEInstallDirs)
    install(FILES hwviewui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/hwview)
  endif()
  install(TARGETS hwview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(
  FILES ${CMAKE_SOURCE_DIR}/data/name-mappings.json
  DESTINATION ${CMAKE_INSTALL_DATADIR}/hwview)

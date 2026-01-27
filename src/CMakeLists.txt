add_subdirectory(backends)
add_subdirectory(common)
add_subdirectory(core)

if(HWVIEW_CLI_ONLY)
  add_executable(hwview main.cpp)
  target_compile_definitions(hwview PRIVATE HWVIEW_HEADLESS)
  target_include_directories(hwview PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(hwview PRIVATE hwview_core Qt6::Core Qt6::Network)
  install(TARGETS hwview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(FILES ${CMAKE_SOURCE_DIR}/man/hwview.1
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
else()
  add_subdirectory(models)
  add_executable(
    hwview WIN32
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

  string(TIMESTAMP YEAR "%Y")
  configure_file(sh.tat.${CMAKE_PROJECT_NAME}.metainfo.xml.in
                 sh.tat.${CMAKE_PROJECT_NAME}.metainfo.xml @ONLY)

  if(NOT FHS OR NON_PORTABLE_MACOS_BUNDLE)
    # macOS bundle, non-portable macOS bundle, or plain Windows (no MSYS, etc).
    if(APPLE)
      set_target_properties(
        hwview
        PROPERTIES OUTPUT_NAME ${DISPLAY_NAME}
                   MACOSX_BUNDLE TRUE
                   MACOSX_BUNDLE_BUNDLE_NAME ${DISPLAY_NAME}
                   MACOSX_BUNDLE_ICON_FILE "hwview.icns"
                   MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
                   MACOSX_BUNDLE_GUI_IDENTIFIER "sh.tat.${CMAKE_PROJECT_NAME}"
                   MACOSX_BUNDLE_COPYRIGHT "Copyright (c) ${YEAR} ${DISPLAY_NAME} authors."
                   MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
      set(deploy_tool_options_arg -hardened-runtime)
      set(executable_path "$<TARGET_FILE_NAME:hwview>.app")
      set(executable_dir ${BUNDLE_INSTALL_PREFIX})
    elseif(WIN32)
      set(deploy_tool_options_arg --no-compiler-runtime)
      set(executable_path "$<TARGET_FILE_NAME:hwview>")
      set(executable_dir "${CMAKE_INSTALL_BINDIR}")
    endif()
    if(NOT NON_PORTABLE_MACOS_BUNDLE OR WIN32)
      qt_generate_deploy_script(
        TARGET hwview
        OUTPUT_SCRIPT deploy_script
        CONTENT [[
qt_deploy_runtime_dependencies(
  EXECUTABLE "${executable_dir}/${executable_path}"
  DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
  GENERATE_QT_CONF
  VERBOSE)
]])
    endif()
    install(TARGETS hwview RUNTIME_DEPENDENCY_SET hwview-deps
            BUNDLE DESTINATION ${BUNDLE_INSTALL_PREFIX})
    if(NOT NON_PORTABLE_MACOS_BUNDLE OR WIN32)
      install(SCRIPT ${deploy_script})
    endif()
    if(APPLE)
      install(
        FILES icons/hwview.icns
        DESTINATION "${BUNDLE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:hwview>.app/Contents/Resources")
      install(
        FILES ../data/name-mappings.json ../data/vendors.json
        DESTINATION "${BUNDLE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:hwview>.app/Contents/Resources")
      if(NOT NON_PORTABLE_MACOS_BUNDLE)
        install(
          FILES ../LICENSE.txt ../README.md ../CHANGELOG.md ../SECURITY.md ../CITATION.cff
          DESTINATION "${BUNDLE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:hwview>.app/Contents/Resources")
      endif()
    elseif(WIN32)
      install(FILES ../LICENSE.txt ../README.md ../CHANGELOG.md ../SECURITY.md TYPE DOC)
      install(FILES ../CITATION.cff DESTINATION ${CMAKE_INSTALL_DATADIR}/hwview)
      if(MINGW)
        get_filename_component(_mingw_bin_dir ${CMAKE_CXX_COMPILER} DIRECTORY)
        install(
          RUNTIME_DEPENDENCY_SET hwview-deps
          PRE_EXCLUDE_REGEXES
            [=[api-ms-]=]
            [=[ext-ms-]=]
            [[kernel32\.dll]]
          POST_EXCLUDE_REGEXES
            [=[.*system32\/.*\.dll]=]
          DIRECTORIES
            ${_mingw_bin_dir}
            $<TARGET_FILE_DIR:hwview>)
      endif()
    endif()
  else()
    # Linux, MSYS package, macOS package without creating a bundle.
    install(TARGETS hwview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.desktop.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/sh.tat.${CMAKE_PROJECT_NAME}.desktop" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sh.tat.${CMAKE_PROJECT_NAME}.desktop"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    foreach(size 16 32 48 64 96 128 256 512)
      install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon_${size}.png"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${size}x${size}/apps"
        RENAME sh.tat.${CMAKE_PROJECT_NAME}.png)
    endforeach()
    install(FILES ../README.md TYPE DOC)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sh.tat.${CMAKE_PROJECT_NAME}.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
    install(FILES ${CMAKE_SOURCE_DIR}/man/hwview.1
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
  endif()

  # CPack configuration
  if(WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/extra.nsh.in ${CMAKE_BINARY_DIR}/extra.nsh @ONLY)
    string(REPLACE "/" "\\\\" CMAKE_BINARY_DIR_ESCAPED ${CMAKE_BINARY_DIR})
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "  !include \\\"${CMAKE_BINARY_DIR_ESCAPED}\\\\extra.nsh\\\"" PARENT_SCOPE)
    set(CPACK_NSIS_DEFINES "RequestExecutionLevel user
!define /date CURRENT_YEAR \\\"%Y\\\"
VIProductVersion \\\"${PROJECT_VERSION}.0\\\"
VIAddVersionKey \\\"ProductName\\\" \\\"${DISPLAY_NAME}\\\"
VIAddVersionKey \\\"CompanyName\\\" \\\"Tatsh\\\"
VIAddVersionKey \\\"FileDescription\\\" \\\"${DISPLAY_NAME} installer.\\\"
VIAddVersionKey \\\"FileVersion\\\" \\\"${PROJECT_VERSION}\\\"
VIAddVersionKey \\\"ProductVersion\\\" \\\"${PROJECT_VERSION}.0\\\"
VIAddVersionKey \\\"LegalCopyright\\\" \\\"Copyright © \\\${CURRENT_YEAR} Tatsh.\\\"
VIAddVersionKey \\\"Comments\\\" \\\"${PROJECT_HOMEPAGE_URL}\\\"" PARENT_SCOPE)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" PARENT_SCOPE)
    set(CPACK_NSIS_CONTACT "https://bsky.app/profile/tatsh.bsky.social" PARENT_SCOPE)
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${CMAKE_PROJECT_NAME}.exe" PARENT_SCOPE)
    set(CPACK_PACKAGE_EXECUTABLES "${CMAKE_PROJECT_NAME};${DISPLAY_NAME}" PARENT_SCOPE)
    set(CPACK_CREATE_DESKTOP_LINKS "${CMAKE_PROJECT_NAME};${DISPLAY_NAME}" PARENT_SCOPE)
    set(CPACK_NSIS_MANIFEST_DPI_AWARE ON PARENT_SCOPE)
    set(CPACK_NSIS_DISPLAY_NAME ${DISPLAY_NAME} PARENT_SCOPE)
    set(CPACK_NSIS_PACKAGE_NAME ${DISPLAY_NAME} PARENT_SCOPE)
    set(CPACK_NSIS_MODIFY_PATH ON PARENT_SCOPE)
    set(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_HOMEPAGE_URL}/issues PARENT_SCOPE)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${DISPLAY_NAME} PARENT_SCOPE)
  endif()
endif()

# Data files for FHS builds only (macOS bundles install to Resources folder above)
if(FHS)
  install(FILES ${CMAKE_SOURCE_DIR}/data/name-mappings.json
                ${CMAKE_SOURCE_DIR}/data/vendors.json
          DESTINATION ${CMAKE_INSTALL_DATADIR}/hwview)
endif()

cmake_minimum_required(VERSION 3.31)
project(hwview LANGUAGES CXX VERSION 0.0.2 DESCRIPTION "Hardware information utility." HOMEPAGE_URL "https://github.com/Tatsh/hwview")

include(FindPkgConfig)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configurable website URL
set(HWVIEW_WEBSITE_URL "https://github.com/Tatsh/hwview" CACHE STRING "Project website URL")
set(DISPLAY_NAME "Hardware Viewer" CACHE STRING "Display name for the application")

option(BUILD_DOCS "Build documentation." OFF)
option(BUILD_DOCS_ONLY "Build documentation only." OFF)

if(BUILD_DOCS OR BUILD_DOCS_ONLY)
  set(DOXYGEN_GENERATE_TAGFILE "${CMAKE_BINARY_DIR}/html/hwview.tags")
  set(DOXYGEN_INTERACTIVE_SVG YES)
  set(DOXYGEN_MARKDOWN_ID_STYLE GITHUB)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
  set(DOXYGEN_TAGFILES
      ./.docs/cppreference-doxygen-web.tag.xml=https://en.cppreference.com/w/
      ./.docs/qtcore.tags=http://doc.qt.io/qt-6/
      ./.docs/qtgui.tags=http://doc.qt.io/qt-6/
      ./.docs/qtwidgets.tags=http://doc.qt.io/qt-6/)
  find_package(Doxygen REQUIRED COMPONENTS dot)
  doxygen_add_docs(docs README.md src ALL)
endif()

if(BUILD_DOCS_ONLY)
  return()
endif()

option(HWVIEW_CLI_ONLY "Build CLI-only version without GUI." OFF)
option(HWVIEW_CXX_WARN_ERROR_ALL "Do not use." OFF)
mark_as_advanced(HWVIEW_CXX_WARN_ERROR_ALL)
option(HWVIEW_USE_KDE "Build with KDE Frameworks integration." OFF)

# Installation options (FHS is always ON for Linux).
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(FHS ON CACHE BOOL "Install using Filesystem Hierarchy Standard layout." FORCE)
else()
  option(FHS "Install using Filesystem Hierarchy Standard layout." OFF)
endif()
option(NON_PORTABLE_MACOS_BUNDLE "Create a non-portable macOS bundle (no Qt deployment)." OFF)

# Bundle install prefix for macOS bundles.
if(APPLE AND NOT FHS)
  if(NOT DEFINED BUNDLE_INSTALL_PREFIX)
    set(BUNDLE_INSTALL_PREFIX "." CACHE STRING "Installation prefix for macOS bundles")
  endif()
endif()

if(HWVIEW_CLI_ONLY)
  find_package(Qt6Core CONFIG REQUIRED)
  set(HWVIEW_USE_KDE OFF)
else()
  find_package(Qt6Concurrent CONFIG REQUIRED)
  find_package(Qt6Network CONFIG REQUIRED)
  find_package(Qt6Widgets CONFIG REQUIRED)
endif()
qt_standard_project_setup()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  pkg_check_modules(UDEV libudev REQUIRED IMPORTED_TARGET)
endif()

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
  find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
  find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
  find_library(DISKARBITRATION_FRAMEWORK DiskArbitration REQUIRED)
endif()

if(HWVIEW_USE_KDE AND NOT HWVIEW_CLI_ONLY)
  find_package(ECM REQUIRED NO_MODULE)
  set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
  find_package(
    KF6
    COMPONENTS ConfigWidgets CoreAddons I18n XmlGui
    REQUIRED)
endif()

add_compile_definitions(HWVIEW_WEBSITE_URL="${HWVIEW_WEBSITE_URL}")

# Sane flags from Kate project.
add_compile_definitions(
  QT_NO_CAST_FROM_ASCII
  QT_NO_CAST_FROM_BYTEARRAY
  QT_NO_CAST_TO_ASCII
  QT_NO_SIGNALS_SLOTS_KEYWORDS
  QT_NO_URL_CAST_FROM_STRING
  QT_STRICT_ITERATORS
  QT_USE_FAST_OPERATOR_PLUS
  QT_USE_QSTRINGBUILDER)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND HWVIEW_CXX_WARN_ERROR_ALL)
    add_compile_options(
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-covered-switch-default # Conflicts with -Wswitch-default.
      -Wno-ctad-maybe-unsupported # Qt's moc uses CTAD without deduction guides.
      -Wno-deprecated-copy-with-user-provided-dtor
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-inconsistent-missing-destructor-override
      -Wno-nrvo # Conflicts with -Wpessimizing-move in some cases.
      -Wno-padded
      -Wno-redundant-parens
      -Wno-unsafe-buffer-usage # Qt moc uses strcmp.
      -Wno-unsafe-buffer-usage-in-libc-call) # Qt moc uses strcmp.
  endif()
endif()

add_subdirectory(src)

# Testing
option(BUILD_TESTS "Build tests." OFF)
option(COVERAGE "Enable code coverage (GCC/Clang only)." OFF)
if(COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
  else()
    message(WARNING "COVERAGE is only supported with GCC or Clang.")
  endif()
endif()
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_VENDOR "Tatsh")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(__cpack_system_name ${CMAKE_SYSTEM_NAME})
if(__cpack_system_name MATCHES "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(__cpack_system_name win64)
  else()
    set(__cpack_system_name win32)
  endif()
elseif(APPLE)
  if(CMAKE_APPLE_SILICON_PROCESSOR)
    set(__cpack_system_name Darwin-arm64)
  else()
    set(__cpack_system_name Darwin-x86_64)
  endif()
endif()
set(CPACK_THREADS 0)
set(CPACK_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${__cpack_system_name})

if(WIN32)
  set(CPACK_GENERATOR NSIS ZIP)
elseif(APPLE)
  if(FHS OR NON_PORTABLE_MACOS_BUNDLE)
    set(CPACK_GENERATOR ZIP)
  else()
    set(CPACK_GENERATOR DragNDrop ZIP)
  endif()
else()
  set(CPACK_GENERATOR ZIP)
endif()
include(CPack)

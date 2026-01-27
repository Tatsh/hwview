cmake_minimum_required(VERSION 3.31)
project(hwview LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "Hardware information utility." HOMEPAGE_URL "https://github.com/Tatsh/hwview")

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
set(DISPLAY_NAME "Hardware View" CACHE STRING "Display name for the application")

option(HWVIEW_CLI_ONLY "Build CLI-only version without GUI." OFF)
option(HWVIEW_CXX_WARN_ERROR_ALL "Do not use." OFF)
option(HWVIEW_USE_KDE "Build with KDE Frameworks integration." OFF)

# Installation options (FHS is always ON for Linux).
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(FHS ON CACHE BOOL "Install using Filesystem Hierarchy Standard layout." FORCE)
else()
  option(FHS "Install using Filesystem Hierarchy Standard layout." ON)
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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  pkg_check_modules(UDEV libudev REQUIRED IMPORTED_TARGET)
endif()

if(APPLE)
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
  set(CPACK_GENERATOR DragNDrop ZIP)
else()
  set(CPACK_GENERATOR ZIP TGZ)
endif()
include(CPack)

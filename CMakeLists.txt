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

option(HWVIEW_CLI_ONLY "Build CLI-only version without GUI." OFF)
option(HWVIEW_CXX_WARN_ERROR_ALL "Do not use." OFF)
option(HWVIEW_USE_KDE "Build with KDE Frameworks integration." OFF)

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

# Pass website URL to code
add_compile_definitions(HWVIEW_WEBSITE_URL="${HWVIEW_WEBSITE_URL}")

# Sane flags from Kate project
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

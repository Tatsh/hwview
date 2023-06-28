cmake_minimum_required(VERSION 3.7.0)
project(devmgmt)

include(FindPkgConfig)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
  Qt5
  COMPONENTS Network Widgets
  REQUIRED)
pkg_check_modules(UDEV libudev REQUIRED IMPORTED_TARGET)

# Sane flags from Kate project
add_definitions(
  -DQT_NO_CAST_FROM_ASCII
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_STRICT_ITERATORS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_USE_QSTRINGBUILDER)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND DEVMGMT_CXX_WARN_ERROR_ALL)
    add_compile_options(
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-deprecated-copy-with-user-provided-dtor
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-inconsistent-missing-destructor-override
      -Wno-padded
      -Wno-redundant-parens)
  endif()
endif()

include_directories(.)
add_executable(
  devmgmt
  deviceinfo.cpp
  main.cpp
  mainwindow.cpp
  models/devbytypemodel.cpp
  models/node.cpp
  propertiesdialog.cpp
  udev/udevenumerate.cpp
  udev/udevmanager.cpp)
target_link_libraries(devmgmt PkgConfig::UDEV Qt5::Network Qt5::Widgets)

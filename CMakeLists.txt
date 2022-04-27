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
  COMPONENTS Widgets
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
add_executable(
  devmgmt
  deviceinfo.cpp
  main.cpp
  mainwindow.cpp
  models/devbytypemodel.cpp
  models/node.cpp
  propertiesdialog.cpp
  udev.cpp)
target_link_libraries(devmgmt Qt5::Widgets PkgConfig::UDEV)

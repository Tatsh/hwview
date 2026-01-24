add_executable(
  devmgmtviewer
  exporteddata.cpp
  main.cpp
  viewerdevbyconnmodel.cpp
  viewerdevbydrivermodel.cpp
  viewerdevbytypemodel.cpp
  viewerdriverdetailsdialog.cpp
  viewerdrvbydevmodel.cpp
  viewerdrvbytypemodel.cpp
  viewermainwindow.cpp
  viewerpropertiesdialog.cpp
  viewerresbyconnmodel.cpp
  viewerresbytypemodel.cpp
  # Shared base classes from models (without DeviceCache dependencies)
  ../models/basetreemodel.cpp
  ../models/node.cpp
  # Shared utilities
  ../namemappings.cpp
  # Shared resources from main app
  ../devmgmt.qrc)

target_include_directories(
  devmgmtviewer
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/..
          ${CMAKE_CURRENT_SOURCE_DIR}/../models)

target_link_libraries(
  devmgmtviewer
  PRIVATE Qt6::Concurrent
          Qt6::Network
          Qt6::Widgets)

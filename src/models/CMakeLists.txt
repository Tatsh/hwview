add_library(
  hwview_models STATIC
  basetreemodel.cpp
  devbyconnmodel.cpp
  devbydrivermodel.cpp
  devbytypemodel.cpp
  drvbydevmodel.cpp
  drvbytypemodel.cpp
  node.cpp
  resbyconnmodel.cpp
  resbytypemodel.cpp)

target_include_directories(hwview_models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(hwview_models PUBLIC hwview_common Qt6::Widgets)

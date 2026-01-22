add_library(
  devmgmt_models STATIC
  basetreemodel.cpp
  devbyconnmodel.cpp
  devbydrivermodel.cpp
  devbytypemodel.cpp
  drvbydevmodel.cpp
  drvbytypemodel.cpp
  node.cpp
  resbyconnmodel.cpp
  resbytypemodel.cpp)

target_include_directories(devmgmt_models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(devmgmt_models PUBLIC Qt6::Widgets)

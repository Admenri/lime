cmake_minimum_required(VERSION 3.30)

add_library(rgssx_core)

target_sources(rgssx_core
 PUBLIC
  raygui.c
  config.cc
  bitmap.cc
  drawable.cc
  font.cc
  graphics.cc
  input.cc
  plane.cc
  shader.cc
  sprite.cc
  table.cc
  tilemap.cc
  viewport.cc
  window.cc
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_sources(rgssx_core PUBLIC platform/win32.cc)
endif()

target_link_libraries(rgssx_core PUBLIC raylib)

cmake_minimum_required(VERSION 3.30)

add_executable(rgssx)
target_sources(rgssx
 PUBLIC
  main.cc
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
  target_sources(rgssx PUBLIC platform/win32.cc)
endif()

target_link_libraries(rgssx
 PUBLIC
  raylib
)

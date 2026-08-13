cmake_minimum_required(VERSION 3.30)

add_library(lime_core)

target_sources(lime_core
 PUBLIC
  raygui.c
  profile.cc
  audio.cc
  bitmap.cc
  drawable.cc
  effect.cc
  filesystem.cc
  font.cc
  geometry.cc
  graphics.cc
  input.cc
  plane.cc
  shader.cc
  sprite.cc
  table.cc
  tilemap.cc
  utility.cc
  viewport.cc
  window.cc
)

target_sources(lime_core
 PUBLIC
  font/font_face.cc
  font/text_shaper.cc
  font/font_atlas.cc
  font/glyph_cache.cc
  font/text_renderer.cc
  font/font_manager.cc
  font/font_system.cc
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_sources(lime_core PUBLIC platform/win32.cc)
endif()

target_link_libraries(lime_core PUBLIC raylib magic_enum)
target_link_libraries(lime_core PUBLIC freetype harfbuzz)

target_link_libraries(lime_core PUBLIC physfs-static)
target_compile_definitions(lime_core
 PUBLIC
  PHYSFS_STATIC=1
  MAGIC_ENUM_RANGE_MIN=0
  MAGIC_ENUM_RANGE_MAX=512
)

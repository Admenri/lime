cmake_minimum_required(VERSION 3.30)

add_library(binding_mruby STATIC)
target_link_libraries(binding_mruby PUBLIC mruby rgssx_core zlibstatic)
target_include_directories(binding_mruby
 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_sources(binding_mruby
 PUBLIC
  mruby_main.cc
  core/binding_init.cc
  core/binding_bitmap.cc
  core/binding_color.cc
  core/binding_font.cc
  core/binding_graphics.cc
  core/binding_input.cc
  core/binding_plane.cc
  core/binding_rect.cc
  core/binding_sprite.cc
  core/binding_table.cc
  core/binding_tilemap.cc
  core/binding_tone.cc
  core/binding_viewport.cc
  core/binding_window.cc
)

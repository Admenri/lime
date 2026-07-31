cmake_minimum_required(VERSION 3.30)

add_library(binding_mruby STATIC)
target_link_libraries(binding_mruby PUBLIC mruby)
target_sources(binding_mruby
 PUBLIC
  mruby_main.cc
)

cmake_minimum_required(VERSION 3.30)

add_executable(rgssx main.cc)
target_link_libraries(rgssx PUBLIC rgssx_core binding_mruby)

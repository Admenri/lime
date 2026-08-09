cmake_minimum_required(VERSION 3.30)

add_executable(lime main.cc resource.rc)
target_link_libraries(lime PUBLIC lime_core binding_mruby)

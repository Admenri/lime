cmake_minimum_required(VERSION 3.30)

if(WIN32)
  add_executable(lime WIN32 main.cc resource.rc)
else()
  add_executable(lime main.cc)
endif()
target_link_libraries(lime PUBLIC lime_core binding_mruby)

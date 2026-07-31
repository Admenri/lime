cmake_minimum_required(VERSION 3.30)

project(RGSSX LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(.)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(3rdparty/raylib)
add_subdirectory(3rdparty/mruby-cmake)

add_subdirectory(src)
add_subdirectory(binding)
add_subdirectory(app)

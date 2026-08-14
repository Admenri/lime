cmake_minimum_required(VERSION 3.30)

project(lime LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(
  SUPPORT_FILEFORMAT_PNG
  SUPPORT_FILEFORMAT_JPG
)

add_compile_definitions(
  SUPPORT_TRACELOG=$<IF:$<CONFIG:Release>,0,1>
)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(3rdparty/zlib)
add_subdirectory(3rdparty/raylib)
add_subdirectory(3rdparty/physfs)
add_subdirectory(3rdparty/freetype)
add_subdirectory(3rdparty/harfbuzz)
add_subdirectory(3rdparty/magic_enum)
add_subdirectory(3rdparty/mruby-cmake)

add_subdirectory(src)
add_subdirectory(binding)
add_subdirectory(app)

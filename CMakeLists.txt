cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# set(VCPKG_TARGET_TRIPLET "x64-windows-static")

project(osu-kps
  VERSION 1.0.0
  DESCRIPTION "KPS (keys per second) indicator for osu! in C++"
  HOMEPAGE_URL "https://github.com/UnnamedOrange/osu-kps"
  LANGUAGES CXX
)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  message(FATAL_ERROR "osu-kps ONLY provides support for MSVC.")
endif()

add_subdirectory("source")

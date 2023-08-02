cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

################################################################
## Dependencies
include(FetchContent)

# GoogleTest
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

# JsonCpp
find_package(jsoncpp REQUIRED CONFIG)

################################################################

file(
  GLOB_RECURSE
  SOURCES
  CONFIGURE_DEPENDS
  "src/*"
)
set(TESTED_SOURCES # Add sources to be test here.
  "utils/ConvertCode.hpp"
  "utils/WindowsResource.hpp"

  "Resource.rc"
  "resource.h"
)
foreach(SOURCE ${TESTED_SOURCES})
  list(APPEND SOURCES "../source/src/${SOURCE}")
endforeach()

add_executable(test-osu-kps)
target_compile_definitions(test-osu-kps PRIVATE UNICODE _UNICODE)
target_compile_features(test-osu-kps PRIVATE cxx_std_20)
target_compile_options(test-osu-kps PRIVATE /utf-8)
target_compile_options(test-osu-kps PRIVATE /W4 /permissive /WX)
target_sources(test-osu-kps PRIVATE "${SOURCES}")
target_include_directories(test-osu-kps PRIVATE "../source/src")

target_link_libraries(test-osu-kps PRIVATE GTest::gtest GTest::gtest_main)
target_link_libraries(test-osu-kps PRIVATE jsoncpp_lib)

gtest_discover_tests(test-osu-kps)

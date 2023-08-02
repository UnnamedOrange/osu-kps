cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

################################################################
## Dependencies
include(FetchContent)

# memory-reader
FetchContent_Declare(
  memory-reader
  GIT_REPOSITORY https://github.com/UnnamedOrange/memory-reader
  GIT_TAG v0.3.0
)
FetchContent_MakeAvailable(memory-reader)

# JsonCpp
find_package(jsoncpp REQUIRED CONFIG)

################################################################

file(
  GLOB_RECURSE
  SOURCES
  CONFIGURE_DEPENDS
  "src/*"
)

add_executable(osu-kps)
target_compile_definitions(osu-kps PRIVATE UNICODE _UNICODE)
target_compile_features(osu-kps PRIVATE cxx_std_20)
target_compile_options(osu-kps PRIVATE /utf-8)
target_compile_options(osu-kps PRIVATE /W4 /permissive) # /WX
target_link_options(osu-kps PRIVATE /SUBSYSTEM:WINDOWS)
# set_target_properties(osu-kps PROPERTIES
#   MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
# )
target_sources(osu-kps PRIVATE "${SOURCES}")

target_compile_definitions(osu-kps PRIVATE
  OSU_KPS_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  OSU_KPS_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  OSU_KPS_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

target_link_libraries(osu-kps PRIVATE
  memory-reader
  jsoncpp_lib
)

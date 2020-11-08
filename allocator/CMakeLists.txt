cmake_minimum_required(VERSION 3.10.2)

#project info
project(Allocator VERSION 1.0.0)

#include dir
set (INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
message("include dir: ${INCLUDE_DIR}")
include_directories(${INCLUDE_DIR})

#source dir
#aux_source_directory("${CMAKE_SOURCE_DIR/src}" SRC_LIST)
file(GLOB_RECURSE SRC_LIST RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")
message("source files: ${SRC_LIST}")
add_executable(allocator ${SRC_LIST})

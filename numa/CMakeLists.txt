cmake_minimum_required(VERSION 3.0.0)
project(numa VERSION 0.1.0)

add_executable(cpp main.cpp)

target_link_libraries(cpp numa)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
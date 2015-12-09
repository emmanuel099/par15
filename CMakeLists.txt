cmake_minimum_required(VERSION 2.8)

project(stencil)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${PROJECT_SOURCE_DIR})

add_definitions(
    -Wall
    -pedantic
    -Wextra
    -O3
    #-DNDEBUG
)

add_subdirectory(stencil)

add_subdirectory(stencil_sequential)
add_subdirectory(stencil_openmp)

cmake_minimum_required(VERSION 2.8)

project(stencil)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TEST_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)
include_directories(${PROJECT_SOURCE_DIR})

add_definitions(
    -Wall
    -pedantic
    -Wextra
    -Ofast
    -msse2
    -DNDEBUG
)

enable_testing()

function(test name command)
    FILE(GLOB inFiles "${TEST_DIRECTORY}/*.test")
    FOREACH(file ${inFiles})
        get_filename_component(file_name ${file} NAME)
        add_test(NAME ${name}_${file_name}
            COMMAND ${CMAKE_COMMAND}
            -Dinput=${file}
            -Dtest_cmd=${command}
            -Dtest_dir=${TEST_DIRECTORY}
            -P ${CMAKE_SOURCE_DIR}/CMake/run_test.cmake
        )
    ENDFOREACH(file)
endfunction(test)

add_subdirectory(stencil)

add_subdirectory(stencil_sequential)
add_subdirectory(stencil_openmp)
add_subdirectory(stencil_cilk)
add_subdirectory(stencil_mpi)

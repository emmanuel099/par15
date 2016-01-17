project(stencil_openmp)

find_package(OpenMP REQUIRED)

if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -lm")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(STENCIL_OPENMP_SRCS
    stencil_openmp.c
    main.c
)

add_executable(stencil_openmp
    ${STENCIL_OPENMP_SRCS}
)

target_link_libraries(stencil_openmp
    stencil
)

# ---------- benchmark ---------- #

add_executable(openmp_benchmark_tmp_matrix
    benchmark.c
    stencil_openmp.c
)
target_link_libraries(openmp_benchmark_tmp_matrix
    stencil
)

add_executable(openmp_benchmark_one_vector
    benchmark.c
    stencil_openmp.c
)
target_link_libraries(openmp_benchmark_one_vector
    stencil
)

set_target_properties(openmp_benchmark_tmp_matrix PROPERTIES COMPILE_FLAGS "-DSTENCIL_TMP_MATRIX")
set_target_properties(openmp_benchmark_one_vector PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR")

# ---------- unit tests ---------- #

add_executable(unit_test_openmp_one_vec
    stencil_openmp.c
    unit_test_one_vec.c
)

target_link_libraries(unit_test_openmp_one_vec
    stencil
)

add_executable(unit_test_openmp_tmp_matrix
    stencil_openmp.c
    unit_test_tmp_matrix.c
)

target_link_libraries(unit_test_openmp_tmp_matrix
    stencil
)

test("openmp_one_vec" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_one_vec)
test("openmp_tmp_matrix" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_tmp_matrix)

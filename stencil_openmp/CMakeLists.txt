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

add_executable(openmp_benchmark_one_vector_tld
    benchmark.c
    stencil_openmp.c
)
target_link_libraries(openmp_benchmark_one_vector_tld
    stencil
)

add_executable(openmp_benchmark_one_vector_colwise
    benchmark.c
    stencil_openmp.c
)
target_link_libraries(openmp_benchmark_one_vector_colwise
    stencil
)

add_executable(openmp_benchmark_one_vector_colwise_tld
    benchmark.c
    stencil_openmp.c
)
target_link_libraries(openmp_benchmark_one_vector_colwise_tld
    stencil
)

add_executable(openmp_benchmark_one_vector_blockwise_tld
    benchmark.c
    stencil_openmp.c
)
target_link_libraries(openmp_benchmark_one_vector_blockwise_tld
    stencil
)

set_target_properties(openmp_benchmark_tmp_matrix PROPERTIES COMPILE_FLAGS "-DSTENCIL_TMP_MATRIX")
set_target_properties(openmp_benchmark_one_vector PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR")
set_target_properties(openmp_benchmark_one_vector_tld PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_TLD")
set_target_properties(openmp_benchmark_one_vector_colwise PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_COLWISE")
set_target_properties(openmp_benchmark_one_vector_colwise_tld PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_COLWISE_TLD")
set_target_properties(openmp_benchmark_one_vector_blockwise_tld PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_BLOCKWISE_TLD")

# ---------- unit tests ---------- #

add_executable(unit_test_openmp_one_vec
    stencil_openmp.c
    test.c
)
target_link_libraries(unit_test_openmp_one_vec
    stencil
)

add_executable(unit_test_openmp_one_vec_tld
    stencil_openmp.c
    test.c
)
target_link_libraries(unit_test_openmp_one_vec_tld
    stencil
)

add_executable(unit_test_openmp_tmp_matrix
    stencil_openmp.c
    test.c
)
target_link_libraries(unit_test_openmp_tmp_matrix
    stencil
)

add_executable(unit_test_openmp_one_vec_colwise
    stencil_openmp.c
    test.c
)
target_link_libraries(unit_test_openmp_one_vec_colwise
    stencil
)

add_executable(unit_test_openmp_one_vec_colwise_tld
    stencil_openmp.c
    test.c
)
target_link_libraries(unit_test_openmp_one_vec_colwise_tld
    stencil
)

add_executable(unit_test_openmp_one_vec_blockwise_tld
    stencil_openmp.c
    test.c
)
target_link_libraries(unit_test_openmp_one_vec_blockwise_tld
    stencil
)

set_target_properties(unit_test_openmp_tmp_matrix PROPERTIES COMPILE_FLAGS "-DSTENCIL_TMP_MATRIX")
set_target_properties(unit_test_openmp_one_vec PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR")
set_target_properties(unit_test_openmp_one_vec_tld PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_TLD")
set_target_properties(unit_test_openmp_one_vec_colwise PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_COLWISE")
set_target_properties(unit_test_openmp_one_vec_colwise_tld PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_COLWISE_TLD")
set_target_properties(unit_test_openmp_one_vec_blockwise_tld PROPERTIES COMPILE_FLAGS "-DSTENCIL_ONE_VECTOR_BLOCKWISE_TLD")

test("openmp_one_vec" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_one_vec)
test("openmp_one_vec_tld" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_one_vec_tld)
test("openmp_tmp_matrix" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_tmp_matrix)
test("openmp_one_vec_colwise" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_one_vec_colwise)
test("openmp_one_vec_colwise_tld" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_one_vec_colwise_tld)
test("openmp_one_vec_blockwise_tld" ${CMAKE_BINARY_DIR}/stencil_openmp/unit_test_openmp_one_vec_blockwise_tld)
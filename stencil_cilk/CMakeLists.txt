project(stencil_cilk)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcilkplus -lm")

add_executable(stencil_cilk
    main.c
    stencil_cilk.c
)
target_link_libraries(stencil_cilk
    stencil
)

# ---------- benchmark ---------- #

add_executable(cilk_benchmark
    benchmark.c
    stencil_cilk.c
)
target_link_libraries(cilk_benchmark
    stencil
)

# ---------- tests ---------- #

add_executable(unit_test_cilk_one_vec_tld
    unit_test_one_vec_tld.c
    stencil_cilk.c
)
target_link_libraries(unit_test_cilk_one_vec_tld
    stencil
)

add_executable(unit_test_cilk_one_vec
    unit_test_one_vec.c
    stencil_cilk.c
)
target_link_libraries(unit_test_cilk_one_vec
    stencil
)

add_executable(unit_test_cilk_two_vec
    unit_test_two_vec.c
    stencil_cilk.c
)
target_link_libraries(unit_test_cilk_two_vec
    stencil
)

add_executable(unit_test_cilk_tmp_matrix
    unit_test_tmp_matrix.c
    stencil_cilk.c
)
target_link_libraries(unit_test_cilk_tmp_matrix
    stencil
)

test("cilk_one_vec_tld" ${CMAKE_BINARY_DIR}/stencil_cilk/unit_test_cilk_one_vec_tld)
test("cilk_one_vec" ${CMAKE_BINARY_DIR}/stencil_cilk/unit_test_cilk_one_vec)
test("cilk_two_vec" ${CMAKE_BINARY_DIR}/stencil_cilk/unit_test_cilk_two_vec)
test("cilk_tmp_matrix" ${CMAKE_BINARY_DIR}/stencil_cilk/unit_test_cilk_tmp_matrix)
project(stencil_sequential)

set(STENCIL_SEQUENTIAL_SRCS
    stencil_sequential.c
    main.c
)

add_executable(stencil_sequential
    ${STENCIL_SEQUENTIAL_SRCS}
)

target_link_libraries(stencil_sequential
    stencil
)

# ---------- benchmark ---------- #

add_executable(sequential_benchmark
    stencil_sequential.c
    benchmark.c
)

target_link_libraries(sequential_benchmark
    stencil
)

# ---------- unit tests ---------- #

add_executable(unit_test_sequential_one_vec
    stencil_sequential.c
    unit_test_one_vec.c
)

target_link_libraries(unit_test_sequential_one_vec
    stencil
)

add_executable(unit_test_sequential_two_vec
    stencil_sequential.c
    unit_test_two_vec.c
)

target_link_libraries(unit_test_sequential_two_vec
    stencil
)

add_executable(unit_test_sequential_tmp_matrix
    stencil_sequential.c
    unit_test_tmp_matrix.c
)

target_link_libraries(unit_test_sequential_tmp_matrix
    stencil
)

test("sequential_one_vec" ${CMAKE_BINARY_DIR}/stencil_sequential/unit_test_sequential_one_vec)
test("sequential_two_vec" ${CMAKE_BINARY_DIR}/stencil_sequential/unit_test_sequential_two_vec)
test("sequential_tmp_matrix" ${CMAKE_BINARY_DIR}/stencil_sequential/unit_test_sequential_tmp_matrix)
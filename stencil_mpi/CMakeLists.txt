project(stencil_mpi)

find_package(MPI REQUIRED)

if(MPI_C_FOUND)
    set(CMAKE_C_COMPILE_FLAGS "${CMAKE_C_COMPILE_FLAGS} ${MPI_C_COMPILE_FLAGS}")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} ${MPI_C_LINK_FLAGS}")
endif()

add_executable(stencil_mpi
    main.c
    stencil_mpi.c
)
target_link_libraries(stencil_mpi
    stencil
    ${MPI_LIBRARIES}
)

add_executable(unit_test_mpi_one_vec
    unit_test_one_vec.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi_one_vec
    stencil
    ${MPI_LIBRARIES}
)

add_executable(unit_test_mpi_two_vec
    unit_test_two_vec.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi_two_vec
    stencil
    ${MPI_LIBRARIES}
)

add_executable(unit_test_mpi_tmp_matrix
    unit_test_tmp_matrix.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi_tmp_matrix
    stencil
    ${MPI_LIBRARIES}
)

mpi_test("mpi_one_vec" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi_one_vec")
mpi_test("mpi_two_vec" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi_two_vec")
mpi_test("mpi_tmp_matrix" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi_tmp_matrix")

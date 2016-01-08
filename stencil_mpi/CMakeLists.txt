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

add_executable(unit_test_mpi
    unit_test_mpi.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi
    stencil
    ${MPI_LIBRARIES}
)

mpi_test("mpi_stencil" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi")

project(stencil_mpi)

execute_process(COMMAND "hostname" OUTPUT_VARIABLE hostname)
string(STRIP ${hostname} hostname)
if (hostname STREQUAL "jupiter.par.tuwien.ac.at")
    include_directories(/opt/mpich/include)
    link_directories(/opt/mpich/lib)
    set(DCMAKE_CC_COMPILER "/opt/mpich/bin/mpicc")
    set(MPI_LIBRARIES "mpich;mpl;pthread")
else()
    find_package(MPI REQUIRED)

    if(MPI_C_FOUND)
        set(CMAKE_C_COMPILE_FLAGS "${CMAKE_C_COMPILE_FLAGS} ${MPI_C_COMPILE_FLAGS}")
        set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} ${MPI_C_LINK_FLAGS}")
    endif()
endif()

add_executable(stencil_mpi
    main.c
    stencil_mpi.c
)
target_link_libraries(stencil_mpi
    stencil
    ${MPI_LIBRARIES}
)

# ---------- benchmarks ---------- #

add_executable(mpi_benchmark_sendrecv
    benchmark_sendrecv.c
    stencil_mpi.c
)
target_link_libraries(mpi_benchmark_sendrecv
    stencil
    ${MPI_LIBRARIES}
)

add_executable(mpi_benchmark_onesided
    benchmark_onesided.c
    stencil_mpi.c
)
target_link_libraries(mpi_benchmark_onesided
    stencil
    ${MPI_LIBRARIES}
)

add_executable(mpi_benchmark_nonblocking
    benchmark_nonblocking.c
    stencil_mpi.c
)
target_link_libraries(mpi_benchmark_nonblocking
    stencil
    ${MPI_LIBRARIES}
)

# ---------- unit test ---------- #

add_executable(unit_test_mpi
    unit_test_mpi.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi
    stencil
    ${MPI_LIBRARIES}
)

mpi_test("mpi_stencil" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi")

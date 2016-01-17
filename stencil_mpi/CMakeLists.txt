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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lm")

add_executable(stencil_mpi
    main.c
    stencil_mpi.c
)
target_link_libraries(stencil_mpi
    stencil
    ${MPI_LIBRARIES}
)

set_target_properties(stencil_mpi PROPERTIES COMPILE_FLAGS "-DSENDRECV_BOUNDARY_EXCHANGE")

# ---------- benchmarks ---------- #

add_executable(mpi_benchmark_sendrecv
    benchmark.c
    stencil_mpi.c
)
target_link_libraries(mpi_benchmark_sendrecv
    stencil
    ${MPI_LIBRARIES}
)

add_executable(mpi_benchmark_onesided
    benchmark.c
    stencil_mpi.c
)
target_link_libraries(mpi_benchmark_onesided
    stencil
    ${MPI_LIBRARIES}
)

add_executable(mpi_benchmark_nonblocking
    benchmark.c
    stencil_mpi.c
)
target_link_libraries(mpi_benchmark_nonblocking
    stencil
    ${MPI_LIBRARIES}
)

set_target_properties(mpi_benchmark_sendrecv PROPERTIES COMPILE_FLAGS "-DSENDRECV_BOUNDARY_EXCHANGE")
set_target_properties(mpi_benchmark_onesided PROPERTIES COMPILE_FLAGS "-DONESIDED_BOUNDARY_EXCHANGE")
set_target_properties(mpi_benchmark_nonblocking PROPERTIES COMPILE_FLAGS "-DNONBLOCKING_BOUNDARY_EXCHANGE")


# ---------- unit test ---------- #

add_executable(unit_test_mpi_sendrecv
    unit_test_mpi.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi_sendrecv
    stencil
    ${MPI_LIBRARIES}
)

add_executable(unit_test_mpi_onesided
    unit_test_mpi.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi_onesided
    stencil
    ${MPI_LIBRARIES}
)

add_executable(unit_test_mpi_nonblocking
    unit_test_mpi.c
    stencil_mpi.c
)
target_link_libraries(unit_test_mpi_nonblocking
    stencil
    ${MPI_LIBRARIES}
)

set_target_properties(unit_test_mpi_sendrecv PROPERTIES COMPILE_FLAGS "-DSENDRECV_BOUNDARY_EXCHANGE")
set_target_properties(unit_test_mpi_onesided PROPERTIES COMPILE_FLAGS "-DONESIDED_BOUNDARY_EXCHANGE")
set_target_properties(unit_test_mpi_nonblocking PROPERTIES COMPILE_FLAGS "-DNONBLOCKING_BOUNDARY_EXCHANGE")

mpi_test("mpi_stencil_sendrecv" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi_sendrecv")
mpi_test("mpi_stencil_onesided" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi_onesided")
mpi_test("mpi_stencil_nonblocking" "${CMAKE_BINARY_DIR}/stencil_mpi/unit_test_mpi_nonblocking")

project(stencil_mpi)

find_package(MPI REQUIRED)

if(MPI_C_FOUND)
    set(CMAKE_C_COMPILE_FLAGS "${CMAKE_C_COMPILE_FLAGS} ${MPI_C_COMPILE_FLAGS}")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} ${MPI_C_LINK_FLAGS}")
endif()

set(STENCIL_MPI_SRCS
    main.c
)

add_executable(stencil_mpi
    ${STENCIL_MPI_SRCS}
)

target_link_libraries(stencil_mpi
    stencil
    ${MPI_LIBRARIES}
)

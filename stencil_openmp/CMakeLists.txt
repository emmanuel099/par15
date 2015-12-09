project(stencil_openmp)

find_package(OpenMP REQUIRED)

if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(STENCIL_OPENMP_SRCS
    main.c
)

add_executable(stencil_openmp
    ${STENCIL_OPENMP_SRCS}
)

target_link_libraries(stencil_openmp
    stencil
)

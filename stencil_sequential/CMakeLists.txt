project(stencil_sequential)

set(STENCIL_SEQUENTIAL_SRCS
    main.c
)

add_executable(stencil_sequential
    ${STENCIL_SEQUENTIAL_SRCS}
)

target_link_libraries(stencil_sequential
    stencil
)

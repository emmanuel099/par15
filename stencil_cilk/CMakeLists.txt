project(stencil_cilk)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcilkplus")

set(STENCIL_CILK_SRCS
    main.c
)

add_executable(stencil_cilk
    ${STENCIL_CILK_SRCS}
)

target_link_libraries(stencil_cilk
    stencil
)

set(STENCIL_LIB_HEADERS
    matrix.h
    vector.h
    util.h
)

set(STENCIL_LIB_SRCS
    matrix.c
    vector.c
    util.c
)

add_library(stencil
    ${STENCIL_LIB_SRCS}
)

install(TARGETS stencil DESTINATION bin)
install(FILES ${STENCIL_LIB_HEADERS} DESTINATION include)

#ifndef __STENCIL_CILK_H
#define __STENCIL_CILK_H

#include <stencil/matrix.h>

double five_point_stencil_host(stencil_matrix_t *matrix, size_t iterations);
void five_point_stencil_client();

#endif // __STENCIL_CILK_H
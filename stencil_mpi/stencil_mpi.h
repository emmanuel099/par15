#ifndef __STENCIL_CILK_H
#define __STENCIL_CILK_H

#include "stencil/matrix.h"
#include "stencil/vector.h"
#include "stencil/util.h"

int stencil_init(int *argc, char ***argv);
int stencil_finalize();

double five_point_stencil_host(stencil_matrix_t *matrix, size_t iterations);
void five_point_stencil_client();

#endif // __STENCIL_CILK_H
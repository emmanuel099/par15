#ifndef __STENCIL_CILK_H
#define __STENCIL_CILK_H

#include "stencil/matrix.h"
#include "stencil/vector.h"
#include "stencil/util.h"


double five_point_stencil_host(stencil_matrix_t *matrix, const size_t iterations, const size_t nr_workers);
void five_point_stencil_client(int rank);

#endif // __STENCIL_CILK_H
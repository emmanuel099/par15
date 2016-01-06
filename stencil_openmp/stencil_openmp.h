#ifndef __STENCIL_OPENMP
#define __STENCIL_OPENMP

#include <stencil/matrix.h>

double five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix);
double five_point_stencil_with_one_vector(stencil_matrix_t *matrix);

#endif // __STENCIL_OPENMP

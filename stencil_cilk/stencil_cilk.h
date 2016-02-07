#ifndef __STENCIL_CILK_H
#define __STENCIL_CILK_H

#include "stencil/matrix.h"
#include "stencil/vector.h"
#include "stencil/util.h"

/**
 * calculates at first the first row of the area which is assigned to each worker
 * each worker buffers this vector and calculates then the other values
 * at the end the buffered vector is copied back to the matrix.
 *
 * buffers one vector during the the sequential calculation on each worker
 *
 * @param matrix matrix
 * @return returns the needed time for the calculation in msec
 */
double cilk_stencil_one_vector(stencil_matrix_t *matrix, const size_t iterations);

/**
 * calculates at first the first row of the area which is assigned to each worker
 * each worker buffers this vector and calculates then the other values
 * at the end the buffered vector is copied back to the matrix.
 *
 * buffers two vectors during the the sequential calculation on each worker
 *
 * @param matrix matrix
 * @return returns the needed time for the calculation in msec
 */
double cilk_stencil_two_vectors(stencil_matrix_t *matrix, const size_t iterations);

/**
 * calculates at first the first row of the area which is assigned to each worker
 * each worker buffers this vector and calculates then the other values
 * at the end the buffered vector is copied back to the matrix.
 *
 * buffers the whole submatrix during the the sequential calculation on each worker
 *
 * @param matrix matrix
 * @return returns the needed time for the calculation in msec
 */
double cilk_stencil_tmp_matrix(stencil_matrix_t *matrix, const size_t iterations);


double cilk_stencil_one_vector_tld(stencil_matrix_t *matrix, const size_t iterations);


#endif // __STENCIL_CILK_H
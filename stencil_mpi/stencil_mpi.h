#ifndef __STENCIL_CILK_H
#define __STENCIL_CILK_H

#include "stencil/matrix.h"
#include "stencil/vector.h"
#include "stencil/util.h"


double mpi_stencil_one_vector_host(stencil_matrix_t *matrix, const size_t nr_workers);
void mpi_stencil_one_vector_client(const size_t rank);

double mpi_stencil_two_vectors_host(stencil_matrix_t *matrix, const size_t nr_workers);
void mpi_stencil_two_vectors_client(const size_t rank);

double mpi_stencil_tmp_matrix_host(stencil_matrix_t *matrix, const size_t nr_workers);
void mpi_stencil_tmp_matrix_client(const size_t rank);

#endif // __STENCIL_CILK_H
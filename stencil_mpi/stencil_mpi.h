#ifndef __STENCIL_CILK_H
#define __STENCIL_CILK_H

#include "stencil/matrix.h"
#include "stencil/vector.h"
#include "stencil/util.h"

#include <mpi.h>

int stencil_init(int *argc, char ***argv, MPI_Comm *comm_card);
int stencil_finalize();

double five_point_stencil_host(stencil_matrix_t *matrix, size_t iterations, MPI_Comm comm_card);
void five_point_stencil_client(MPI_Comm comm_card);

#endif // __STENCIL_CILK_H
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include <stencil/matrix.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    stencil_matrix_t *matrix = stencil_matrix_new(4, 4, 1);
    stencil_matrix_free(matrix);

    MPI_Finalize();

    return EXIT_SUCCESS;
}

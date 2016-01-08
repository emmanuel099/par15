#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "stencil_mpi.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    const size_t rows = 10000 + 2;   // n + 2 boundary vectors
    const size_t cols = 20000 + 2;   // m + 2 boundary vectors
    stencil_matrix_t *matrix;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        matrix = stencil_matrix_new(rows, cols, 1);
        printf("\nfive_point_stencil_with_one_vector: \n");
    }
    for (int i = 0; i < 6; i++) {
        if (rank == 0) {
            double time = five_point_stencil_host(matrix, 6, size);
            printf("elapsed time %fms\n", time);
        } else {
            five_point_stencil_client(rank);
        }
    }

    /* free memory */
    if (rank == 0) {
        stencil_matrix_free(matrix);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}

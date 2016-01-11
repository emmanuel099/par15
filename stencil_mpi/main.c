#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "stencil_mpi.h"

#define MASTER 0

int main(int argc, char **argv)
{
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        return EXIT_FAILURE;
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == MASTER) {
        fprintf(stdout, "Hello mpi\n");

        stencil_matrix_t *matrix = stencil_matrix_new(10000, 20000, 1);
        if (matrix == NULL) {
            return EXIT_FAILURE;
        }

        printf("\nfive_point_stencil_with_one_vector: \n");
        for (int i = 0; i < 3; i++) {
            double time = five_point_stencil_host(matrix, 6);
            printf("elapsed time %fms\n", time);
        }

        stencil_matrix_free(matrix);
    } else {
        five_point_stencil_client();
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}

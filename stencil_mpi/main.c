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

    stencil_matrix_t *matrix = NULL;
    if (rank == MASTER) {
        fprintf(stdout, "Hello mpi\n");

        matrix = stencil_matrix_new(10000, 20000, 1);
        if (matrix == NULL) {
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (rank == MASTER) {
            double time = five_point_stencil_host(matrix, 6);
            if (time < 0.0) {
                stencil_matrix_free(matrix);
                MPI_Finalize();
                return EXIT_FAILURE;
            }

            printf("elapsed time %fms\n", time);
        } else {
            five_point_stencil_client();
        }
    }

    if (rank == MASTER) {
        stencil_matrix_free(matrix);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}

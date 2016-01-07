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
            double time = mpi_stencil_one_vector_host(matrix, size);
            printf("elapsed time %fms\n", time);
        } else {
            mpi_stencil_one_vector_client(rank);
        }
    }

    if (rank == 0) {
        printf("\nfive_point_stencil_with_two_vectors: \n");
    }
    for (int i = 0; i < 6; i++) {
        if (rank == 0) {
            double time = mpi_stencil_two_vectors_host(matrix, size);
            printf("elapsed time %fms\n", time);
        } else {
            mpi_stencil_two_vectors_client(rank);
        }
    }


    if (rank == 0) {
        printf("\nfive_point_stencil_with_tmp_matrix: \n");
    }
    for (size_t i = 0; i < 6; i++) {
        if (rank == 0) {
            double time = mpi_stencil_tmp_matrix_host(matrix, size);
            printf("elapsed time %fms\n", time);
        } else {
            mpi_stencil_tmp_matrix_client(rank);
        }
    }

    /* free memory */
    if (rank == 0) {
        stencil_matrix_free(matrix);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}

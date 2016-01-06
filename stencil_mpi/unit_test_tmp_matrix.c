#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <mpi.h>

#include "stencil_mpi.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL) {
        fprintf(stderr, "ERROR: file argument missing");
        return EXIT_FAILURE;
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        stencil_matrix_t *matrix = new_matrix_from_file(argv[1]);
        if (matrix == NULL) {
            return EXIT_FAILURE;
        }

        mpi_stencil_tmp_matrix_host(matrix, size);

        matrix_to_file(matrix, stdout);
        stencil_matrix_free(matrix);
    } else {
        mpi_stencil_tmp_matrix_client(rank);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

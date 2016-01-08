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

    stencil_init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        stencil_matrix_t *matrix = new_matrix_from_file(argv[1]);
        if (matrix == NULL) {
            return EXIT_FAILURE;
        }

        five_point_stencil_host(matrix, 6);

        matrix_to_file(matrix, stdout);
        stencil_matrix_free(matrix);
    } else {
        five_point_stencil_client(rank);
    }

    stencil_finalize();

    return EXIT_SUCCESS;
}

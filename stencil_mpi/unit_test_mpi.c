#include <stdio.h>
#include <stdlib.h>

#include <stencil/util.h>

#include "stencil_mpi.h"

#define MASTER 0

int main(int argc, char **argv)
{
    if (argv[1] == NULL) {
        fprintf(stderr, "ERROR: file argument missing");
        return EXIT_FAILURE;
    }

    MPI_Comm comm_card;
    stencil_init(&argc, &argv, &comm_card);

    int rank;
    MPI_Comm_rank(comm_card, &rank);

    if (rank == MASTER) {
        stencil_matrix_t *matrix = new_matrix_from_file(argv[1]);
        if (matrix == NULL) {
            return EXIT_FAILURE;
        }

        five_point_stencil_host(matrix, 5, comm_card);

        matrix_to_file(matrix, stdout);
        stencil_matrix_free(matrix);
    } else {
        five_point_stencil_client(comm_card);
    }

    stencil_finalize();

    return EXIT_SUCCESS;
}

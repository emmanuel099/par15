#include <stdlib.h>

#include <stencil/util.h>

#include "stencil_openmp/stencil_openmp.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL) {
        fprintf(stdout, "ERROR: file argument missing");
        return EXIT_FAILURE;
    }

    stencil_matrix_t *matrix = new_matrix_from_file(argv[1]);
    if (matrix == NULL) {
        return EXIT_FAILURE;
    }
    five_point_stencil_with_tmp_matrix(matrix, 5);
    matrix_to_file(matrix, stdout);

    stencil_matrix_free(matrix);
    return EXIT_SUCCESS;
}
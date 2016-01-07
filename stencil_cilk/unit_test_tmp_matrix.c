#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "stencil/util.h"
#include "stencil_cilk.h"

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
    cilk_stencil_tmp_matrix(matrix, 1);
    matrix_to_file(matrix, stdout);

    free(matrix);
    return EXIT_SUCCESS;
}
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "stencil_cilk.h"

int main(int argc, char **argv)
{
    const size_t rows = 10000 + 2;   // n + 2 boundary vectors
    const size_t cols = 20000 + 2;   // m + 2 boundary vectors

    __cilkrts_set_param("nworkers", "2");
    stencil_matrix_t *matrix = stencil_matrix_new(rows, cols, 1);

    printf("\nfive_point_stencil_with_one_vector: \n");
    for (int i = 0; i < 3; i++) {
        double time = cilk_stencil_one_vector(matrix, 5);
        printf("elapsed time %fms\n", time);
    }

    printf("\nfive_point_stencil_with_two_vectors: \n");
    for (int i = 0; i < 3; i++) {
        double time = cilk_stencil_two_vectors(matrix, 6);
        printf("elapsed time %fms\n", time);
    }

    printf("\nfive_point_stencil_with_tmp_matrix: \n");
    for (size_t i = 0; i < 3; i++) {
        double time = cilk_stencil_tmp_matrix(matrix, 6);
        printf("elapsed time %fms\n", time);
    }

    /* free memory */
    stencil_matrix_free(matrix);

    return EXIT_SUCCESS;
}

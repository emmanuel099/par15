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

    stencil_matrix_t *matrix = new_randomized_matrix(rows, cols, 0, 1000);

    /* start computations */
    printf("five_point_stencil_with_one_vector_buffer_first_row: \n");
    for (int i = 0; i < 6; i++) {
        double time = cilk_stencil_buffer_first_row(matrix);
        printf("elapsed time %fms\n", time);
    }

    printf("\nfive_point_stencil_with_one_vector: \n");
    for (int i = 0; i < 6; i++) {
        double time = cilk_stencil_one_vector(matrix);
        printf("elapsed time %fms\n", time);
    }

    printf("\nfive_point_stencil_with_two_vectors: \n");
    for (int i = 0; i < 6; i++) {
        double time = cilk_stencil_two_vectors(matrix);
        printf("elapsed time %fms\n", time);
    }

    printf("\nfive_point_stencil_with_tmp_matrix: \n");
    for (size_t i = 0; i < 6; i++) {
        double time = cilk_stencil_two_vectors(matrix);
        printf("elapsed time %fms\n", time);
    }

    /* free memory */
    stencil_matrix_free(matrix);

    return EXIT_SUCCESS;
}

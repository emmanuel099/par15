#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <stencil/util.h>
#include <stencil/matrix.h>
#include <stencil/vector.h>
#include <stencil/util.h>

#include "stencil_sequential/stencil_sequential.h"

int main(int argc, char **argv)
{
    fprintf(stdout, "Hello sequential\n");

    stencil_matrix_t *matrix = stencil_matrix_new(10000, 20000, 1);
    if (matrix == NULL) {
        return EXIT_FAILURE;
    }

    printf("\nfive_point_stencil_with_tmp_matrix: \n");
    for (int i = 0; i < 3; i++) {
        double time = five_point_stencil_with_tmp_matrix(matrix, 6);
        printf("elapsed time %fms\n", time);
    }

    printf("\nfive_point_stencil_with_one_vector: \n");
    for (int i = 0; i < 3; i++) {
        double time = five_point_stencil_with_one_vector(matrix, 6);
        printf("elapsed time %fms\n", time);
    }


    printf("\nfive_point_stencil_with_two_vectors: \n");
    for (int i = 0; i < 3; i++) {
        double time = five_point_stencil_with_two_vectors(matrix, 6);
        printf("elapsed time %fms\n", time);
    }

    stencil_matrix_free(matrix);

    return EXIT_SUCCESS;
}

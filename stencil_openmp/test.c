#include <stdlib.h>

#include <stencil/util.h>

#include "stencil_openmp/stencil_openmp.h"

#define TEST_ITERATIONS 5

//#define STENCIL_ONE_VECTOR
//#define STENCIL_ONE_VECTOR_TLD
//#define STENCIL_TMP_MATRIX

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
#if defined(STENCIL_ONE_VECTOR)
    five_point_stencil_with_one_vector(matrix, TEST_ITERATIONS);
#elif defined(STENCIL_ONE_VECTOR_TLD)
    five_point_stencil_with_one_vector_tld(matrix, TEST_ITERATIONS);
#elif defined(STENCIL_TMP_MATRIX)
    five_point_stencil_with_tmp_matrix(matrix, TEST_ITERATIONS);
#elif defined(STENCIL_ONE_VECTOR_COLWISE)
    five_point_stencil_with_one_vector_columnwise(matrix, TEST_ITERATIONS);
#elif defined(STENCIL_ONE_VECTOR_COLWISE_TLD)
    five_point_stencil_with_one_vector_columnwise_tld(matrix, TEST_ITERATIONS);
#endif
    matrix_to_file(matrix, stdout);

    stencil_matrix_free(matrix);
    return EXIT_SUCCESS;
}
#include <stdlib.h>

#include <stencil/util.h>

#include "stencil_openmp/stencil_openmp.h"

int main(int argc, char **argv)
{
    if (argc < 5) {
        return EXIT_FAILURE;
    }

    size_t rows = strtol(argv[1], NULL, 10);
    size_t cols = strtol(argv[2], NULL, 10);
    size_t iterations = strtol(argv[3], NULL, 10);
    size_t threads = strtol(argv[4], NULL, 10);

    omp_set_num_threads(threads);

    stencil_matrix_t *matrix = new_randomized_matrix(rows, cols, 1, 0, 100);
    if (matrix == NULL) {
        return EXIT_FAILURE;
    }

    double sum = 0.0;
    for (int i = 0; i < 30; i++) {
        sum += five_point_stencil_with_one_vector(matrix, iterations);
    }

    double elapsed_time = sum / 30.0;
    fprintf(stdout, "%f", elapsed_time);

    stencil_matrix_free(matrix);
    return EXIT_SUCCESS;
}
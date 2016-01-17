#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "stencil/util.h"
#include "stencil_sequential/stencil_sequential.h"

#define BENCHMARK_ITERATIONS 30

//#define STENCIL_ONE_VECTOR
//#define STENCIL_TMP_MATRIX

int main(int argc, char **argv)
{
    if (argc < 4) {
        return EXIT_FAILURE;
    }

    size_t rows = strtol(argv[1], NULL, 10);
    size_t cols = strtol(argv[2], NULL, 10);
    size_t iterations = strtol(argv[3], NULL, 10);

    stencil_matrix_t *matrix = new_randomized_matrix(rows, cols, 1, 0, 100);
    if (matrix == NULL) {
        return EXIT_FAILURE;
    }

    double min = DBL_MAX;
    double max = DBL_MIN;
    double sum = 0.0;
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
#if defined(STENCIL_ONE_VECTOR)
        const double elapsed_time = five_point_stencil_with_one_vector(matrix, iterations);
#elif defined(STENCIL_TMP_MATRIX)
        const double elapsed_time = five_point_stencil_with_tmp_matrix(matrix, iterations);
#endif
        min = fmin(min, elapsed_time);
        max = fmax(max, elapsed_time);
        sum += elapsed_time;
    }

    const double avg = sum / BENCHMARK_ITERATIONS;
    fprintf(stdout, "%f;%f;%f", min, avg, max);

    stencil_matrix_free(matrix);
    return EXIT_SUCCESS;
}
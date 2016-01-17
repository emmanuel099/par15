#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "stencil/util.h"
#include "stencil_cilk.h"

#define BENCHMARK_ITERATIONS 30

int main(int argc, char **argv)
{
    if (argc < 5) {
        return EXIT_FAILURE;
    }

    size_t rows = strtol(argv[1], NULL, 10);
    size_t cols = strtol(argv[2], NULL, 10);
    size_t iterations = strtol(argv[3], NULL, 10);

    __cilkrts_set_param("nworkers", argv[4]);

    stencil_matrix_t *matrix = new_randomized_matrix(rows, cols, 1, 0, 100);
    if (matrix == NULL) {
        return EXIT_FAILURE;
    }

    double min = DBL_MAX;
    double max = DBL_MIN;
    double sum = 0.0;
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        const double elapsed_time = cilk_stencil_one_vector(matrix, iterations);
        min = fmin(min, elapsed_time);
        max = fmax(max, elapsed_time);
        sum += elapsed_time;
    }

    const double avg = sum / BENCHMARK_ITERATIONS;
    fprintf(stdout, "%f;%f;%f", min, avg, max);

    stencil_matrix_free(matrix);
    return EXIT_SUCCESS;
}
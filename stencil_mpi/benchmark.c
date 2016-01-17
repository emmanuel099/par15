#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <mpi.h>

#include <stencil/util.h>

#include "stencil_mpi.h"

#define MASTER 0
#define BENCHMARK_ITERATIONS 30

int main(int argc, char **argv)
{
    if (argc < 4) {
        return EXIT_FAILURE;
    }

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        return EXIT_FAILURE;
    }

    size_t rows = strtol(argv[1], NULL, 10);
    size_t cols = strtol(argv[2], NULL, 10);
    size_t iterations = strtol(argv[3], NULL, 10);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == MASTER) {
        stencil_matrix_t *matrix = new_randomized_matrix(rows, cols, 1, 0, 100);
        if (matrix == NULL) {
            return EXIT_FAILURE;
        }

        double min = DBL_MAX;
        double max = DBL_MIN;
        double sum = 0.0;
        for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
            const double elapsed_time = five_point_stencil_host(matrix, iterations);
            min = fmin(min, elapsed_time);
            max = fmax(max, elapsed_time);
            sum += elapsed_time;
        }

        const double avg = sum / BENCHMARK_ITERATIONS;
        fprintf(stdout, "%f;%f;%f", min, avg, max);

        stencil_matrix_free(matrix);
    } else {
        for (int i = 0; i < 30; i++) {
            five_point_stencil_client();
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

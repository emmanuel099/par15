#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "stencil_cilk.h"

inline double stencil_five_point_kernel(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    return (stencil_matrix_get(matrix, row - 1, col) +
            stencil_matrix_get(matrix, row, col - 1) +
            stencil_matrix_get(matrix, row, col + 1) +
            stencil_matrix_get(matrix, row + 1, col)) * 0.25;
}

static void five_point_stencil_for_row(const stencil_matrix_t *matrix, const stencil_vector_t *vector, const size_t row)
{
    const size_t cols = matrix->cols - matrix->boundary;

    for (size_t col = matrix->boundary; col < cols; col++) {
        stencil_vector_set(vector, col, stencil_five_point_kernel(matrix, row, col));
    }
}

static void five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix, const size_t start_row, const size_t rows)
{
    if (rows <= 0) {
        return;
    }

    const size_t end_row = start_row + rows - 1;
    const size_t cols = matrix->cols - matrix->boundary;

    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, start_row - 1, 0, rows + 1, matrix->cols, 0);

    for (size_t row = start_row; row < end_row; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            stencil_matrix_set(matrix, row, col, stencil_five_point_kernel(tmp_matrix, row - start_row + 1, col));
        }
    }

    stencil_matrix_free(tmp_matrix);
}

static void five_point_stencil_with_two_vectors(stencil_matrix_t *matrix, const size_t start_row, const size_t rows)
{
    if (rows <= 0) {
        return;
    }

    const size_t end_row = start_row + rows - 1;
    const size_t cols = matrix->cols - matrix->boundary;

    stencil_vector_t *above = stencil_vector_new(matrix->cols);
    stencil_vector_t *current = stencil_vector_new(matrix->cols);

    // calculate the first rows
    five_point_stencil_for_row(matrix, above, start_row);

    // calculate the remaining rows
    for (size_t row = start_row + 1; row < end_row; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            stencil_vector_set(current, col, value);
        }

        stencil_matrix_set_row(matrix, row - 1, above);
        stencil_vector_t *tmp = above;
        above = current;
        current = tmp;
    }

    // copy back calculated values of the last non-boundary row
    stencil_matrix_set_row(matrix, end_row - 1, above);

    stencil_vector_free(above);
    stencil_vector_free(current);
}

static void five_point_stencil_with_one_vector(stencil_matrix_t *matrix, const size_t start_row, const size_t rows)
{
    if (rows <= 0) {
        return;
    }

    const size_t cols = matrix->cols - matrix->boundary;
    const size_t end_row = start_row + rows - 1;

    // calculate first row
    stencil_vector_t *tmp = stencil_vector_new(matrix->cols);
    five_point_stencil_for_row(matrix, tmp, start_row);

    // calculate the remaining rows
    for (size_t row = start_row + 1; row < end_row; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }
    // copy back the last row
    stencil_matrix_set_row(matrix, end_row - 1, tmp);

    stencil_vector_free(tmp);
}

static double run_parallel(stencil_matrix_t *matrix, const size_t iterations, void (*stencil_sequential)(stencil_matrix_t*, const size_t, const size_t))
{
    const size_t boundary = matrix->boundary * 2;
    const size_t workers = __cilkrts_get_nworkers();
    const size_t rows_per_worker = (matrix->rows - boundary) / workers; // rows per worker without 2 overlapping rows
    stencil_vector_t **first_row_vectors = malloc(workers * sizeof(stencil_vector_t*));

    // create tmp vectors
    for (size_t i = 0; i < workers; i++) {
        first_row_vectors[i] = stencil_vector_new(matrix->cols);
    }

    double t1 = get_time();

    for (size_t it = 0; it < iterations; it++) {
        // calculate first row on each worker and buffer values
        for (size_t i = 0; i < workers; i++) {
            cilk_spawn five_point_stencil_for_row(matrix, first_row_vectors[i], i * rows_per_worker + matrix->boundary);
        }
        cilk_sync;

        // calculate other values
        for (size_t i = 0; i < (workers - 1); i++) {
            const size_t start_row = i * rows_per_worker + matrix->boundary + 1;
            cilk_spawn stencil_sequential(matrix, start_row, rows_per_worker);
        }
        // last worker calculates more than rows_per_worker if row % workers != 0
        const size_t start_row = (workers - 1) * rows_per_worker + matrix->boundary + 1;
        stencil_sequential(matrix, start_row, rows_per_worker + (matrix->rows - boundary) % workers);
        cilk_sync;

        // copy first row vectors values back to matrix
        cilk_for (size_t i = 0; i < workers; i++) {
            stencil_matrix_set_row(matrix, i * rows_per_worker + matrix->boundary, first_row_vectors[i]);
        }
    }
    
    double t2 = get_time();

    // free memory
    for (size_t i = 0; i < workers; i++) {
        stencil_vector_free(first_row_vectors[i]);
    }
    free(first_row_vectors);

    return t2 - t1;
}

double cilk_stencil_one_vector(stencil_matrix_t *matrix, const size_t iterations)
{
    return run_parallel(matrix, iterations, five_point_stencil_with_one_vector);
}

double cilk_stencil_two_vectors(stencil_matrix_t *matrix, const size_t iterations)
{
    return run_parallel(matrix, iterations, five_point_stencil_with_two_vectors);
}

double cilk_stencil_tmp_matrix(stencil_matrix_t *matrix, const size_t iterations)
{
    return run_parallel(matrix, iterations, five_point_stencil_with_tmp_matrix);
}
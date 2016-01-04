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

static void five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix)
{
    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, 0, 0, matrix->rows, matrix->cols);

    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    for (size_t row = 1; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(tmp_matrix, row, col);
            stencil_matrix_set(matrix, row, col, value);
        }
    }

    stencil_matrix_free(tmp_matrix);
}

static void five_point_stencil_with_one_vector(stencil_matrix_t *matrix)
{
    stencil_vector_t *tmp = stencil_matrix_get_row(matrix, 0);

    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    for (size_t row = 1; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }

    // copy back the last row (without boundary! therefore do not use stencil_matrix_set_row)
    memcpy(stencil_matrix_get_ptr(matrix, rows - 1, 1), tmp->values + 1, (matrix->cols - 2) * sizeof(double));

    stencil_vector_free(tmp);
}

static void five_point_stencil_with_two_vectors(stencil_matrix_t *matrix)
{
    stencil_vector_t *above = stencil_matrix_get_row(matrix, 0);
    stencil_vector_t *current = stencil_vector_new(matrix->cols);

    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    for (size_t row = 1; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            stencil_vector_set(current, col, value);
        }


        // copy back row values (without boundary! therefore do not use stencil_matrix_set_row)
        memcpy(stencil_matrix_get_ptr(matrix, row - 1, 1), above->values + 1, (matrix->cols - 2) * sizeof(double));
        stencil_vector_t *tmp = above;
        above = current;
        current = tmp;
    }

    // copy back the last row (without boundary! therefore do not use stencil_matrix_set_row)
    memcpy(stencil_matrix_get_ptr(matrix, rows - 1, 1), above->values + 1, (matrix->cols - 2) * sizeof(double));

    stencil_vector_free(above);
    stencil_vector_free(current);
}

static stencil_vector_t* calc_first_row(stencil_matrix_t *matrix, size_t row)
{
    stencil_vector_t *vector = stencil_matrix_get_row(matrix, row);
    const size_t cols = matrix->cols - 1;

    for (size_t col = 1; col < cols; col++) {
        const double value = stencil_five_point_kernel(matrix, row, col);
        stencil_vector_set(vector, col, value);
    }

    return vector;
}

static void update_values(stencil_matrix_t *matrix, size_t start_row, size_t end_row)
{
    const size_t rows = end_row - 1;
    const size_t cols = matrix->cols - 1;

    if (start_row >= end_row) {
        return;
    }

    stencil_vector_t *tmp = stencil_matrix_get_row(matrix, start_row - 1);

    for (size_t row = start_row; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }

    // copy back the last row (without boundary! therefore do not use stencil_matrix_set_row)
    memcpy(stencil_matrix_get_ptr(matrix, rows - 1, 1), tmp->values + 1, (matrix->cols - 2) * sizeof(double));

    stencil_vector_free(tmp);
}

double cilk_stencil_buffer_first_row(stencil_matrix_t *matrix)
{
    const size_t workers = __cilkrts_get_nworkers();
    const size_t rows_per_worker = (matrix->rows - 2) / workers; // rows per worker without 2 overlapping rows

    stencil_vector_t **first_row_vectors = malloc(workers * sizeof(stencil_vector_t*));

    struct timeval t1;
    gettimeofday(&t1, NULL);

    for (size_t i = 0; i < workers; i++) {
        first_row_vectors[i] = cilk_spawn calc_first_row(matrix, i * rows_per_worker + 1);
    }
    cilk_sync;

    /* calculate other values */
    for (size_t i = 0; i < workers; i++) {
        size_t start_row = i * rows_per_worker + 2;
        size_t end_row = start_row + rows_per_worker;
        cilk_spawn update_values(matrix, start_row, end_row);
    }
    cilk_sync;

    /* copy first row vectors values back to matrix*/
    for (size_t i = 0; i < workers; i++) {
        stencil_matrix_set_row(matrix, i * rows_per_worker + 1, first_row_vectors[i]);
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    return time_difference_ms(t1, t2);
}

static double run_parallel(stencil_matrix_t *matrix, void (*stencil_sequential)(stencil_matrix_t *matrix))
{
    const size_t workers = __cilkrts_get_nworkers();
    const size_t rows_per_worker = (matrix->rows - 2) / workers; // rows per worker without 2 overlapping rows

    /* create submatrices */
    stencil_matrix_t **submatrices = malloc(workers * sizeof(stencil_matrix_t*));
    for (size_t i = 0; i < workers; i++) {
        submatrices[i] = stencil_matrix_get_submatrix(matrix, i * rows_per_worker, 0, rows_per_worker + 2, matrix->cols);
    }

    struct timeval t1;
    gettimeofday(&t1, NULL);

    /* calculate submatrices */
    for (size_t i = 0; i < workers; i++) {
        cilk_spawn stencil_sequential(submatrices[i]);
    }
    cilk_sync;

    /* copy values from submatrices back to main matrix */
    int matrix_row = 1;
    for (size_t i = 0; i < workers; i++) {
        for (size_t row = 1; row <= (submatrices[i]->rows - 2); ++row) {
            memcpy(stencil_matrix_get_ptr(matrix, matrix_row, 0), stencil_matrix_get_ptr(submatrices[i], row, 0), submatrices[i]->cols * sizeof(double));
            ++matrix_row;
        }
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    /* free memory */
    for (size_t i = 0; i < workers; i++) {
        stencil_matrix_free(submatrices[i]);
    }
    free(submatrices);

    return time_difference_ms(t1, t2);
}

double cilk_stencil_one_vector(stencil_matrix_t *matrix)
{
    return run_parallel(matrix, five_point_stencil_with_one_vector);
}

double cilk_stencil_two_vectors(stencil_matrix_t *matrix)
{
    return run_parallel(matrix, five_point_stencil_with_two_vectors);
}

double cilk_stencil_tmp_matrix(stencil_matrix_t *matrix)
{
    return run_parallel(matrix, five_point_stencil_with_tmp_matrix);
}
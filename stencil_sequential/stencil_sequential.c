#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "stencil/vector.h"
#include "stencil/util.h"

#include "stencil_sequential.h"

inline double stencil_five_point_kernel(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    return (stencil_matrix_get(matrix, row - 1, col) +
            stencil_matrix_get(matrix, row, col - 1) +
            stencil_matrix_get(matrix, row, col + 1) +
            stencil_matrix_get(matrix, row + 1, col)) * 0.25;
}

double five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix)
{
    assert(matrix->boundary >= 1);

    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, 0, 0, matrix->rows, matrix->cols, 0);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    for (size_t row = matrix->boundary; row < rows; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(tmp_matrix, row, col);
            stencil_matrix_set(matrix, row, col, value);
        }
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    stencil_matrix_free(tmp_matrix);

    return time_difference_ms(t1, t2);
}

double five_point_stencil_with_two_vectors(stencil_matrix_t *matrix)
{
    assert(matrix->boundary >= 1);

    stencil_vector_t *above = stencil_matrix_get_row(matrix, matrix->boundary - 1);
    stencil_vector_t *current = stencil_vector_new(matrix->cols);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    for (size_t row = matrix->boundary; row < rows; row++) {
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
    stencil_matrix_set_row(matrix, rows - 1, above);

    struct timeval t2;
    gettimeofday(&t2, NULL);

    stencil_vector_free(above);
    stencil_vector_free(current);

    return time_difference_ms(t1, t2);
}

double five_point_stencil_with_one_vector(stencil_matrix_t *matrix)
{
    assert(matrix->boundary >= 1);

    stencil_vector_t *tmp = stencil_matrix_get_row(matrix, matrix->boundary - 1);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    for (size_t row = matrix->boundary; row < rows; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }

    // copy back calculated values of the last non-boundary row
    stencil_matrix_set_row(matrix, rows - 1, tmp);

    struct timeval t2;
    gettimeofday(&t2, NULL);

    stencil_vector_free(tmp);
    
    return time_difference_ms(t1, t2);
}
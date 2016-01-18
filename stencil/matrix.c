#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "matrix.h"

double *stencil_matrix_get_ptr(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);

    return matrix->values + row * matrix->cols + col;
}

stencil_matrix_t *stencil_matrix_new(size_t rows, size_t cols, size_t boundary)
{
    assert(boundary >= 0);
    assert(rows >= 2 * boundary);
    assert(cols >= 2 * boundary);

    const size_t len = rows * cols;

    double *values = (double *)malloc(len * sizeof(double));
    if (!values) {
        goto exit_values;
    }

    stencil_matrix_t *matrix = (stencil_matrix_t *)malloc(sizeof(stencil_matrix_t));
    if (!matrix) {
        goto exit_matrix;
    }
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->boundary = boundary;
    matrix->values = values;

    return matrix;

exit_matrix:
    free(values);
exit_values:
    return NULL;
}

void stencil_matrix_free(stencil_matrix_t *matrix)
{
    if (!matrix) {
        return;
    }

    free(matrix->values);
    free(matrix);
}

void stencil_matrix_set_row(const stencil_matrix_t *matrix, size_t row, const stencil_vector_t *const vector)
{
    assert(matrix);
    assert(vector);
    assert(matrix->boundary <= row && row < (matrix->rows - matrix->boundary));
    assert(matrix->cols == stencil_vector_size(vector));

    double *src = stencil_vector_get_ptr(vector, matrix->boundary);
    double *dest = stencil_matrix_get_ptr(matrix, row, matrix->boundary);
    memcpy(dest, src, (matrix->cols - 2 * matrix->boundary) * sizeof(double));
}

stencil_vector_t *stencil_matrix_get_row(const stencil_matrix_t *matrix, size_t row)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);

    stencil_vector_t *vector = stencil_vector_new(matrix->cols);
    if (!vector) {
        return NULL;
    }

    double *src = stencil_matrix_get_ptr(matrix, row, 0);
    double *dest = stencil_vector_get_ptr(vector, 0);
    memcpy(dest, src, matrix->cols * sizeof(double));

    return vector;
}

stencil_matrix_t *stencil_matrix_get_submatrix(const stencil_matrix_t *const matrix, size_t row, size_t col, size_t rows, size_t cols, size_t boundary)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);
    assert(0 < rows && rows <= (matrix->rows - row));
    assert(0 < cols && cols <= (matrix->cols - col));

    stencil_matrix_t *submatrix = stencil_matrix_new(rows, cols, boundary);
    if (!submatrix) {
        return NULL;
    }

    for (size_t i = row, j = 0; j < rows; i++, j++) {
        double *src = stencil_matrix_get_ptr(matrix, i, col);
        double *dest = stencil_matrix_get_ptr(submatrix, j, 0);
        memcpy(dest, src, cols * sizeof(double));
    }

    return submatrix;
}

bool stencil_matrix_equals(const stencil_matrix_t *const matrix1, const stencil_matrix_t *const matrix2)
{
    assert(matrix1);
    assert(matrix2);

    if ((matrix1->rows != matrix2->rows) || (matrix1->cols != matrix2->cols)) {
        return false;
    }

    double *values1 = matrix1->values;
    double *values2 = matrix2->values;

    const size_t len = matrix1->rows * matrix1->cols;
    for (size_t i = 0; i < len; i++) {
        if (fabs(values1[i] - values2[i]) >= DBL_EPSILON) {
            return false;
        }
    }

    return true;
}

void stencil_matrix_print(const stencil_matrix_t *const matrix)
{
    assert(matrix);

    for (size_t i = 0; i < matrix->rows; i++) {
        printf("   %zu: [", (i + 1));
        if (matrix->cols > 0) {
            printf("%0.2f", stencil_matrix_get(matrix, i, 0));
        }
        for (size_t j = 1; j < matrix->cols; j++) {
            printf(", %0.2f", stencil_matrix_get(matrix, i, j));
        }
        printf("]\n");
    }
}
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "matrix.h"

static double *stencil_matrix_get_ptr(stencil_matrix_t *matrix, size_t row, size_t col)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);

    return matrix->values + row * matrix->cols + col;
}

stencil_matrix_t *stencil_matrix_new(size_t rows, size_t cols)
{
    assert(rows >= 0);
    assert(cols >= 0);

    const size_t len = rows * cols;

    double *values = (double *)malloc(len * sizeof(double));
    if (!values) {
        goto exit_values;
    }
    memset(values, 0, len * sizeof(double));

    stencil_matrix_t *matrix = (stencil_matrix_t *)malloc(sizeof(stencil_matrix_t));
    if (!matrix) {
        goto exit_matrix;
    }
    matrix->rows = rows;
    matrix->cols = cols;
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

stencil_matrix_t *stencil_matrix_get_submatrix(stencil_matrix_t *matrix, size_t row, size_t col, size_t rows, size_t cols)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);
    assert(0 <= rows && rows < (matrix->rows - row));
    assert(0 <= cols && cols < (matrix->cols - col));

    stencil_matrix_t *submatrix = stencil_matrix_new(rows, cols);
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

bool stencil_matrix_equals(stencil_matrix_t *matrix1, stencil_matrix_t *matrix2)
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

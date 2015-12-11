#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"

struct stencil_matrix {
    size_t rows;
    size_t cols;
    double *values;
};

static inline size_t to_index(stencil_matrix_t *matrix, size_t row, size_t col)
{
    return row * matrix->cols + col;
}

stencil_matrix_t *stencil_matrix_new(size_t rows, size_t cols)
{
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
    free(matrix);
exit_values:
    fprintf(stderr, "Could not create matrix");
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

double stencil_matrix_get(stencil_matrix_t *matrix, size_t row, size_t col)
{
    assert(matrix);
    assert(row <= matrix->rows);
    assert(col <= matrix->cols);

    return matrix->values[to_index(matrix, row, col)];
}

void stencil_matrix_set(stencil_matrix_t *matrix, size_t row, size_t col, double value)
{
    assert(matrix);
    assert(row <= matrix->rows);
    assert(col <= matrix->cols);

    matrix->values[to_index(matrix, row, col)] = value;
}

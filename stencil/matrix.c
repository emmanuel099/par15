#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"

static inline ssize_t to_index(stencil_matrix_t *matrix, ssize_t row, ssize_t col)
{
    return row * matrix->cols + col;
}

stencil_matrix_t *stencil_matrix_new(ssize_t rows, ssize_t cols)
{
    assert(rows >= 0);
    assert(cols >= 0);

    const ssize_t len = rows * cols;

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

double stencil_matrix_get(stencil_matrix_t *matrix, ssize_t row, ssize_t col)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);

    return matrix->values[to_index(matrix, row, col)];
}

double *stencil_matrix_get_ptr(stencil_matrix_t *matrix, ssize_t row, ssize_t col)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);

    return matrix->values + to_index(matrix, row, col);
}

void stencil_matrix_set(stencil_matrix_t *matrix, ssize_t row, ssize_t col, double value)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);

    matrix->values[to_index(matrix, row, col)] = value;
}

stencil_matrix_t *stencil_matrix_get_submatrix(stencil_matrix_t *matrix, ssize_t row, ssize_t col, ssize_t rows, ssize_t cols)
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

    stencil_matrix_mutable_row_iterator_t *src_it = stencil_matrix_mutable_row_iterator_new(matrix);
    stencil_matrix_row_iterator_skip(src_it, row);

    stencil_matrix_mutable_row_iterator_t *dest_it = stencil_matrix_mutable_row_iterator_new(submatrix);

    while (stencil_matrix_row_iterator_next(src_it) && stencil_matrix_row_iterator_next(dest_it)) {
        memcpy(stencil_matrix_row_get_ptr(dest_it, 0), stencil_matrix_row_get_ptr(src_it, col), cols);
    }

    stencil_matrix_row_iterator_free(src_it);
    stencil_matrix_row_iterator_free(dest_it);

    return submatrix;
}

stencil_matrix_mutable_row_iterator_t *stencil_matrix_mutable_row_iterator_new(stencil_matrix_t *matrix)
{
    assert(matrix);

    stencil_matrix_mutable_row_iterator_t *it = (stencil_matrix_mutable_row_iterator_t *)malloc(sizeof(stencil_matrix_mutable_row_iterator_t));
    if (!it) {
        return NULL;
    }
    it->row = -1;
    it->matrix = matrix;
    it->values = NULL;

    return it;
}

stencil_matrix_mutable_row_iterator_t *stencil_matrix_mutable_row_iterator_next(stencil_matrix_mutable_row_iterator_t *it)
{
    assert(it);

    stencil_matrix_t *matrix = it->matrix;

    it->row++;
    if (it->row < matrix->rows) {
        it->values = stencil_matrix_get_ptr(matrix, it->row, 0);
        return it;
    } else {
        return NULL;
    }
}

void stencil_matrix_mutable_row_iterator_skip(stencil_matrix_mutable_row_iterator_t *it, ssize_t rows)
{
    assert(it);
    assert(rows >= 0);

    it->row += rows;
}

void stencil_matrix_mutable_row_iterator_free(stencil_matrix_mutable_row_iterator_t *it)
{
    if (!it) {
        return;
    }

    free(it);
}

stencil_matrix_copy_row_iterator_t *stencil_matrix_copy_row_iterator_new(stencil_matrix_t *matrix)
{
    assert(matrix);

    double *values = (double *)malloc(matrix->cols * sizeof(double));
    if (!values) {
        goto exit_values;
    }

    stencil_matrix_copy_row_iterator_t *it = (stencil_matrix_copy_row_iterator_t *)malloc(sizeof(stencil_matrix_copy_row_iterator_t));
    if (!it) {
        goto exit_it;
    }
    it->row = -1;
    it->matrix = matrix;
    it->values = values;

    return it;

exit_it:
    free(values);
exit_values:
    return NULL;
}

stencil_matrix_copy_row_iterator_t *stencil_matrix_copy_row_iterator_next(stencil_matrix_copy_row_iterator_t *it)
{
    assert(it);

    stencil_matrix_t *matrix = it->matrix;

    it->row++;
    if (it->row < matrix->rows) {
        memcpy(it->values, stencil_matrix_get_ptr(matrix, it->row, 0), matrix->cols);
        return it;
    } else {
        return NULL;
    }
}

void stencil_matrix_copy_row_iterator_skip(stencil_matrix_copy_row_iterator_t *it, ssize_t rows)
{
    assert(it);
    assert(rows >= 0);

    it->row += rows;
}

void stencil_matrix_copy_row_iterator_free(stencil_matrix_copy_row_iterator_t *it)
{
    if (!it) {
        return;
    }

    free(it->values);
    free(it);
}

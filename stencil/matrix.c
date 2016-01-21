#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

//#define STENCIL_USE_HUGE_PAGES

#ifdef STENCIL_USE_HUGE_PAGES
#include <sys/mman.h>
#endif

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

#ifdef STENCIL_USE_HUGE_PAGES
    double *values = (double *)mmap(0, len * sizeof(double), PROT_READ | PROT_WRITE,
                                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    if (values == MAP_FAILED) {
        goto exit_values;
    }
#else
    double *values = (double *)malloc(len * sizeof(double));
    if (!values) {
        goto exit_values;
    }
#endif

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
#ifdef STENCIL_USE_HUGE_PAGES
    munmap(values, len * sizeof(double));
#else
    free(values);
#endif
exit_values:
    return NULL;
}

void stencil_matrix_free(stencil_matrix_t *matrix)
{
    if (!matrix) {
        return;
    }

#ifdef STENCIL_USE_HUGE_PAGES
    munmap(matrix->values, matrix->rows * matrix->cols * sizeof(double));
#else
    free(matrix->values);
#endif
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

void stencil_matrix_set_column(const stencil_matrix_t *matrix, size_t col, const stencil_vector_t *const vector)
{
    assert(matrix);
    assert(vector);
    assert(matrix->boundary <= col && col < (matrix->cols - matrix->boundary));
    assert(matrix->rows == stencil_vector_size(vector));

    double *src_it = stencil_vector_get_ptr(vector, matrix->boundary);
    double *src_end = stencil_vector_get_ptr(vector, stencil_vector_size(vector) - matrix->boundary - 1) + 1;
    double *dest_it = stencil_matrix_get_ptr(matrix, matrix->boundary, col);
    while (src_it != src_end) {
        *dest_it = *src_it;
        dest_it += matrix->cols;
        ++src_it;
    }
}

stencil_vector_t *stencil_matrix_get_column(const stencil_matrix_t *matrix, size_t col)
{
    assert(matrix);
    assert(0 <= col && col < matrix->cols);

    stencil_vector_t *vector = stencil_vector_new(matrix->rows);
    if (!vector) {
        return NULL;
    }

    double *src_it = stencil_matrix_get_ptr(matrix, 0, col);
    double *src_end = stencil_matrix_get_ptr(matrix, matrix->rows, col);
    double *dest_it = stencil_vector_get_ptr(vector, 0);
    while (src_it != src_end) {
        *dest_it = *src_it;
        src_it += matrix->cols;
        ++dest_it;
    }

    return vector;
}

void stencil_matrix_set_submatrix(const stencil_matrix_t *matrix, size_t row, size_t col, const stencil_matrix_t *const submatrix)
{
    assert(matrix);
    assert(submatrix);
    assert(matrix->boundary <= row && row < (matrix->rows - matrix->boundary));
    assert(matrix->boundary <= col && col < (matrix->cols - matrix->boundary));
    assert((submatrix->rows - 2 * submatrix->boundary) <= (matrix->rows - matrix->boundary - row));
    assert((submatrix->cols - 2 * submatrix->boundary) <= (matrix->cols - matrix->boundary - col));

    const size_t rows = submatrix->rows - submatrix->boundary; // not *2 because we start with an offset = boundary
    const size_t cols = submatrix->cols - 2 * submatrix->boundary;

    for (size_t i = row, j = submatrix->boundary; j < rows; i++, j++) {
        double *src = stencil_matrix_get_ptr(submatrix, j, submatrix->boundary);
        double *dest = stencil_matrix_get_ptr(matrix, i, col);
        memcpy(dest, src, cols * sizeof(double));
    }
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
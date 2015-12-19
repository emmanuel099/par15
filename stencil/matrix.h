#ifndef __STENCIL_MATRIX_H
#define __STENCIL_MATRIX_H

#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

struct stencil_matrix {
    size_t rows;
    size_t cols;
    double *values;
};
typedef struct stencil_matrix stencil_matrix_t;

/**
 * Creates a new matrix of size \a rows rows by \a cols columns.
 *
 * @param rows Number of rows
 * @param cols Number of columns
 *
 * @return A pointer to a matrix initialized with 0 values, NULL on failure.
 */
stencil_matrix_t *stencil_matrix_new(size_t rows, size_t cols);

/**
 * Frees the memory of a matrix \a matrix.
 */
void stencil_matrix_free(stencil_matrix_t *matrix);

/**
 * Returns the value at position [\a row, \a col] from matrix \a matrix.
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param row Row index (must be in range [0, matrix.rows])
 * @param col Column index (must be in range [0, matrix.cols])
 *
 * @return Value at position [\a row, \a col]
 */
inline double stencil_matrix_get(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);

    return matrix->values[row * matrix->cols + col];
}

/**
 * Sets the value at position [\a row, \a col] of matrix \a matrix.
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param row Row index (must be in range [0, matrix.rows])
 * @param col Column index (must be in range [0, matrix.cols])
 * @param value Value which should be set
 */
inline void stencil_matrix_set(const stencil_matrix_t *matrix, size_t row, size_t col, double value)
{
    assert(matrix);
    assert(0 <= row && row < matrix->rows);
    assert(0 <= col && col < matrix->cols);

    matrix->values[row * matrix->cols + col] = value;
}

stencil_matrix_t *stencil_matrix_get_submatrix(const stencil_matrix_t *const matrix, size_t row, size_t col, size_t rows, size_t cols);

/**
 * Tests the two given matrices \a matrix1 and \a matrix2 for equality.
 *
 * @param matrix1 A pointer to the first matrix (must be valid)
 * @param matrix2 A pointer to the second matrix (must be valid)
 *
 * @return True if the matrices are equal, false otherwise.
 */
bool stencil_matrix_equals(const stencil_matrix_t *const matrix1, const stencil_matrix_t *const matrix2);

#endif // __STENCIL_MATRIX_H
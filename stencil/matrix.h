#ifndef __STENCIL_MATRIX_H
#define __STENCIL_MATRIX_H

#include <stddef.h>

struct stencil_matrix;
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
double stencil_matrix_get(stencil_matrix_t *matrix, size_t row, size_t col);

/**
 * Sets the value at position [\a row, \a col] of matrix \a matrix.
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param row Row index (must be in range [0, matrix.rows])
 * @param col Column index (must be in range [0, matrix.cols])
 * @param value Value which should be set
 */
void stencil_matrix_set(stencil_matrix_t *matrix, size_t row, size_t col, double value);

#endif // __STENCIL_MATRIX_H
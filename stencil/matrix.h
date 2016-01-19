#ifndef __STENCIL_MATRIX_H
#define __STENCIL_MATRIX_H

#include <assert.h>
#include <stdbool.h>

#include "vector.h"

struct stencil_matrix {
    size_t rows;
    size_t cols;
    size_t boundary;
    double *values;
};
typedef struct stencil_matrix stencil_matrix_t;

/**
 * Creates a new matrix of size \a rows rows by \a cols columns.
 *
 * @param rows Number of rows with boundary rows
 * @param cols Number of columns with boundary cols
 * @param boundary The size of the boundary (rows and cols)
 *
 * @return A pointer to a matrix, NULL on failure.
 */
stencil_matrix_t *stencil_matrix_new(size_t rows, size_t cols, size_t boundary);

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
 * @note The matrix boundary is immutable!
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param row Row index (must be in range [matrix.boundary, matrix.rows - matrix.boundary])
 * @param col Column index (must be in range [matrix.boundary, matrix.cols - matrix.boundary])
 * @param value Value which should be set
 */
inline void stencil_matrix_set(const stencil_matrix_t *matrix, size_t row, size_t col, double value)
{
    assert(matrix);
    assert(matrix->boundary <= row && row < (matrix->rows - matrix->boundary));
    assert(matrix->boundary <= col && col < (matrix->cols - matrix->boundary));

    matrix->values[row * matrix->cols + col] = value;
}

/**
 * @param matrix matrix
 * @param row row of the field
 * @param col column of the filed
 *
 * @return returns a pointer to the matrix field specified with \a row and \a col
 */
double *stencil_matrix_get_ptr(const stencil_matrix_t *const matrix, size_t row, size_t col);

/**
 * Set the values of row \a row of matrix \a matrix.
 *
 * @note The matrix boundary is immutable, thus boundary values from the vector
 *       are not copied!
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param row Row index (must be in range [matrix.boundary, matrix.rows - matrix.boundary])
 * @param vector A pointer to a vector which contains the values (must be valid and the size
 *               must be equal to matrix.cols)
 */
void stencil_matrix_set_row(const stencil_matrix_t *matrix, size_t row, const stencil_vector_t *const vector);

stencil_vector_t *stencil_matrix_get_row(const stencil_matrix_t *matrix, size_t row);

/**
 * Set the values of column \a col of matrix \a matrix.
 *
 * @note The matrix boundary is immutable, thus boundary values from the vector
 *       are not copied!
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param col Column index (must be in range [matrix.boundary, matrix.cols - matrix.boundary])
 * @param vector A pointer to a vector which contains the values (must be valid and the size
 *               must be equal to matrix.rows)
 */
void stencil_matrix_set_column(const stencil_matrix_t *matrix, size_t col, const stencil_vector_t *const vector);

stencil_vector_t *stencil_matrix_get_column(const stencil_matrix_t *matrix, size_t col);

stencil_matrix_t *stencil_matrix_get_submatrix(const stencil_matrix_t *const matrix, size_t row, size_t col, size_t rows, size_t cols, size_t boundary);

/**
 * Tests the two given matrices \a matrix1 and \a matrix2 for equality.
 *
 * @param matrix1 A pointer to the first matrix (must be valid)
 * @param matrix2 A pointer to the second matrix (must be valid)
 *
 * @return True if the matrices are equal, false otherwise.
 */
bool stencil_matrix_equals(const stencil_matrix_t *const matrix1, const stencil_matrix_t *const matrix2);

/**
 * Prints the values of the matrix \a matrix to stdout (e.g. for debugging).
 *
 * @param matrix A pointer to the matrix (must be valid)
 */
void stencil_matrix_print(const stencil_matrix_t *const matrix);

#endif // __STENCIL_MATRIX_H
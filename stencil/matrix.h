#ifndef __STENCIL_MATRIX_H
#define __STENCIL_MATRIX_H

#include <stddef.h>

struct stencil_matrix;
typedef struct stencil_matrix stencil_matrix_t;

struct stencil_matrix_mutable_row_iterator;
typedef struct stencil_matrix_mutable_row_iterator stencil_matrix_mutable_row_iterator_t;

struct stencil_matrix_copy_row_iterator;
typedef struct stencil_matrix_copy_row_iterator stencil_matrix_copy_row_iterator_t;

/**
 * Creates a new matrix of size \a rows rows by \a cols columns.
 *
 * @param rows Number of rows
 * @param cols Number of columns
 *
 * @return A pointer to a matrix initialized with 0 values, NULL on failure.
 */
stencil_matrix_t *stencil_matrix_new(ssize_t rows, ssize_t cols);

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
double stencil_matrix_get(stencil_matrix_t *matrix, ssize_t row, ssize_t col);

/**
 * Returns a pointer to the element at position [\a row, \a col] of matrix \a matrix.
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param row Row index (must be in range [0, matrix.rows])
 * @param col Column index (must be in range [0, matrix.cols])
 *
 * @return Pointer to the element at position [\a row, \a col]
 */
double *stencil_matrix_get_ptr(stencil_matrix_t *matrix, ssize_t row, ssize_t col);

/**
 * Sets the value at position [\a row, \a col] of matrix \a matrix.
 *
 * @param matrix A pointer to the matrix (must be valid)
 * @param row Row index (must be in range [0, matrix.rows])
 * @param col Column index (must be in range [0, matrix.cols])
 * @param value Value which should be set
 */
void stencil_matrix_set(stencil_matrix_t *matrix, ssize_t row, ssize_t col, double value);


#define stencil_matrix_row_iterator_next(it) _Generic((it), \
        stencil_matrix_mutable_row_iterator_t *: stencil_matrix_mutable_row_iterator_next, \
        stencil_matrix_copy_row_iterator_t *: stencil_matrix_copy_row_iterator_next \
        )(it)

#define stencil_matrix_row_iterator_skip(it, rows) _Generic((it), \
        stencil_matrix_mutable_row_iterator_t *: stencil_matrix_mutable_row_iterator_skip, \
        stencil_matrix_copy_row_iterator_t *: stencil_matrix_copy_row_iterator_skip \
        )(it, rows)

#define stencil_matrix_row_iterator_free(it) _Generic((it), \
        stencil_matrix_mutable_row_iterator_t *: stencil_matrix_mutable_row_iterator_free, \
        stencil_matrix_copy_row_iterator_t *: stencil_matrix_copy_row_iterator_free \
        )(it)

#define stencil_matrix_row_get(row, col) _Generic((row), \
        stencil_matrix_mutable_row_iterator_t *: stencil_matrix_mutable_row_iterator_get, \
        stencil_matrix_copy_row_iterator_t *: stencil_matrix_copy_row_iterator_get \
        )(row, col)

#define stencil_matrix_row_set(row, col, value) _Generic((row), \
        stencil_matrix_mutable_row_iterator_t *: stencil_matrix_mutable_row_iterator_set \
        )(row, col, value)


stencil_matrix_mutable_row_iterator_t *stencil_matrix_mutable_row_iterator_new(stencil_matrix_t *matrix);
stencil_matrix_mutable_row_iterator_t *stencil_matrix_mutable_row_iterator_next(stencil_matrix_mutable_row_iterator_t *it);
void stencil_matrix_mutable_row_iterator_skip(stencil_matrix_mutable_row_iterator_t *it, ssize_t rows);
void stencil_matrix_mutable_row_iterator_free(stencil_matrix_mutable_row_iterator_t *it);
double stencil_matrix_mutable_row_iterator_get(stencil_matrix_mutable_row_iterator_t *it, ssize_t col);
void stencil_matrix_mutable_row_iterator_set(stencil_matrix_mutable_row_iterator_t *it, ssize_t col, double value);

stencil_matrix_copy_row_iterator_t *stencil_matrix_copy_row_iterator_new(stencil_matrix_t *matrix);
stencil_matrix_copy_row_iterator_t *stencil_matrix_copy_row_iterator_next(stencil_matrix_copy_row_iterator_t *it);
void stencil_matrix_copy_row_iterator_skip(stencil_matrix_copy_row_iterator_t *it, ssize_t rows);
void stencil_matrix_copy_row_iterator_free(stencil_matrix_copy_row_iterator_t *it);
double stencil_matrix_copy_row_iterator_get(stencil_matrix_copy_row_iterator_t *it, ssize_t col);

#endif // __STENCIL_MATRIX_H
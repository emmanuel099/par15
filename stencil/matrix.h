#ifndef __STENCIL_MATRIX_H
#define __STENCIL_MATRIX_H

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

// TODO provide setter/getter to access and set the values of a matrix

#endif // __STENCIL_MATRIX_H
#ifndef __STENCIL_MATRIX_H
#define __STENCIL_MATRIX_H

struct stencil_matrix;
typedef struct stencil_matrix stencil_matrix_t;

stencil_matrix_t *stencil_matrix_new(size_t rows, size_t cols);
void stencil_matrix_free(stencil_matrix_t *matrix);

// TODO provide setter/getter to access and set the values of a matrix

#endif // __STENCIL_MATRIX_H
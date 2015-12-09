#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"

struct stencil_matrix {
    int dimensions;
    int *sizes;
    double *values;
};

stencil_matrix_t *stencil_matrix_new(int dimensions, ...)
{
    assert(dimensions >= 2);

    int *sizes = malloc(dimensions * sizeof(int));
    if (!sizes) {
        goto exit;
    }
    int len = 0;
    va_list args;
    va_start(args, dimensions);
    for (int i = 0; i < dimensions; i++) {
        const int size = va_arg(args, int);
        sizes[i] = size;
        len += size;
    }

    double *values = malloc(len * sizeof(double));
    if (!values) {
        goto exit_values;
    }
    memset(values, 0, len * sizeof(double));

    stencil_matrix_t *matrix = malloc(sizeof(stencil_matrix_t));
    if (!matrix) {
        goto exit_matrix;
    }
    matrix->dimensions = dimensions;
    matrix->sizes = sizes;
    matrix->values = values;

    return matrix;

exit_matrix:
    free(matrix);
exit_values:
    free(values);
exit:
    fprintf(stderr, "Could not create matrix");
    return NULL;
}

void stencil_matrix_free(stencil_matrix_t *matrix)
{
    if (!matrix) {
        return;
    }

    free(matrix->sizes);
    free(matrix->values);
    free(matrix);
}

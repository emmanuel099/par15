#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "vector.h"

stencil_vector_t *stencil_vector_new(size_t size)
{
    double *values = (double *)malloc(size * sizeof(double));
    if (!values) {
        goto exit_values;
    }
    memset(values, 0, size * sizeof(double));

    stencil_vector_t *vector = (stencil_vector_t *)malloc(sizeof(stencil_vector_t));
    if (!vector) {
        goto exit_vector;
    }
    vector->size = size;
    vector->values = values;

    return vector;

exit_vector:
    free(values);
exit_values:
    return NULL;
}

void stencil_vector_free(stencil_vector_t *vector)
{
    if (!vector) {
        return;
    }

    free(vector->values);
    free(vector);
}

bool stencil_vector_equals(const stencil_vector_t *const vector1, const stencil_vector_t *const vector2)
{
    assert(vector1);
    assert(vector2);

    if (vector1->size != vector2->size) {
        return false;
    }

    double *values1 = vector1->values;
    double *values2 = vector2->values;

    const size_t size = vector1->size;
    for (size_t i = 0; i < size; i++) {
        if (fabs(values1[i] - values2[i]) >= DBL_EPSILON) {
            return false;
        }
    }

    return true;
}

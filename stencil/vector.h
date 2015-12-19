#ifndef __STENCIL_MATRIX_H
#define __STENCIL_MATRIX_H

#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

struct stencil_vector {
    size_t size;
    double *values;
};
typedef struct stencil_vector stencil_vector_t;

/**
 * Creates a new vector of size \a size.
 *
 * @param size Size of the vector
 *
 * @return A pointer to a vector initialized with 0 values, NULL on failure.
 */
stencil_vector_t *stencil_vector_new(size_t size);

/**
 * Frees the memory of a vector \a vector.
 */
void stencil_vector_free(stencil_vector_t *vector);

/**
 * Returns the value at position \a i from vector \a vector.
 *
 * @param vector A pointer to the vector (must be valid)
 * @param i Value index (must be in range [0, vector.size[)
 *
 * @return Value at position \a i
 */
inline double stencil_vector_get(stencil_vector_t *vector, size_t i)
{
    assert(vector);
    assert(0 <= i && i < vector->size);

    return vector->values[i];
}

/**
 * Sets the value at position \a i of vector \a vector.
 *
 * @param vector A pointer to the vector (must be valid)
 * @param i Value index (must be in range [0, vector.size[)
 * @param value Value which should be set
 */
inline void stencil_vector_set(stencil_vector_t *vector, size_t i, double value)
{
    assert(vector);
    assert(0 <= i && i < vector->size);

    vector->values[i] = value;
}

/**
 * Tests the two given vectors \a vector1 and \a vector2 for equality.
 *
 * @param vector1 A pointer to the first vector (must be valid)
 * @param vector2 A pointer to the second vector (must be valid)
 *
 * @return True if the vectors are equal, false otherwise.
 */
bool stencil_vector_equals(stencil_vector_t *vector1, stencil_vector_t *vector2);

#endif // __STENCIL_MATRIX_H

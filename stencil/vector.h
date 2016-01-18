#ifndef __STENCIL_VECTOR_H
#define __STENCIL_VECTOR_H

#include <stdbool.h>
#include <assert.h>
#include <stdbool.h>

struct stencil_vector {
    bool is_memory_owner;
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
 * Creates a new vector of size \a size but without allocating memory to store
 * the values, the vector will use the given pointer to memory instead.
 *
 * @param values Valid pointer to memory (at least \a size big)
 * @param size Size of the vector
 *
 * @return A pointer to a vector, NULL on failure.
 */
stencil_vector_t *stencil_vector_new_noalloc(double *values, size_t size);

/**
 * Frees the memory of a vector \a vector.
 */
void stencil_vector_free(stencil_vector_t *vector);

inline size_t stencil_vector_size(const stencil_vector_t *const vector)
{
    assert(vector);

    return vector->size;
}

/**
 * Returns the value at position \a i from vector \a vector.
 *
 * @param vector A pointer to the vector (must be valid)
 * @param i Value index (must be in range [0, vector.size[)
 *
 * @return Value at position \a i
 */
inline double stencil_vector_get(const stencil_vector_t *const vector, size_t i)
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
inline void stencil_vector_set(const stencil_vector_t *vector, size_t i, double value)
{
    assert(vector);
    assert(0 <= i && i < vector->size);

    vector->values[i] = value;
}

inline double* stencil_vector_get_ptr(const stencil_vector_t *const vector, size_t i)
{
    assert(vector);
    assert(0 <= i && i < vector->size);

    return vector->values + i;
}

/**
 * Tests the two given vectors \a vector1 and \a vector2 for equality.
 *
 * @param vector1 A pointer to the first vector (must be valid)
 * @param vector2 A pointer to the second vector (must be valid)
 *
 * @return True if the vectors are equal, false otherwise.
 */
bool stencil_vector_equals(const stencil_vector_t *const vector1, const stencil_vector_t *const vector2);

#endif // __STENCIL_VECTOR_H

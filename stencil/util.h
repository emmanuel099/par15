#ifndef __STENCIL_UTIL_H
#define __STENCIL_UTIL_H

#include <assert.h>
#include <stdbool.h>

#include <stencil/matrix.h>

/**
 * creates a matrix from the provided csv file \a filepath
 * the first line of the file has to contain
 * the number of rows and number of columns
 *
 * @param filepath path to the csv file
 *
 * @return returns the matrix or NULL if an error has ocurred
 */
stencil_matrix_t* matrix_from_file(const char* filepath);

#endif // __STENCIL_UTIL_H
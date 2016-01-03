#ifndef __STENCIL_UTIL_H
#define __STENCIL_UTIL_H

#include <assert.h>
#include <stdbool.h>

#include <stencil/matrix.h>

/**
 * creates a new matrix with values from the provided csv file \a filepath
 * the first line of the file has to contain
 * the number of rows and number of columns
 *
 * @param filepath path to the csv file
 *
 * @return returns the matrix if successful or NULL if an error has ocurred
 */
stencil_matrix_t* new_matrix_from_file(const char* filepath);

/**
 * creates a new matrix with randomized values
 * the values are in the interval [\a min_value, \a max_value)
 *
 * @param rows number of rows of the matrix
 * @param cols number of columns of the matrix
 * @param min_value lower end of the value interval
 * @param max_value upper end of the value interval
 *
 * @returns returns the matrix if successful or NULL if an error has ocurred
 */
stencil_matrix_t* new_randomized_matrix(int rows, int cols, int min_value, int max_value);

/**
 * writes the matrix \a matrix to the csv file \a filepath
 *
 * @param matrix matrix to write
 * @param filepath file name
 *
 * @return returns true if the matrix was written successfully
 */
bool matrix_to_file(const stencil_matrix_t* matrix, const char* filepath);

/**
 * @param t1 first time value
 * @param t2 second time value
 *
 * @return returns the time between \a t1 and \a t2 in msecs
 */
double time_difference_ms(struct timeval t1, struct timeval t2);

#endif // __STENCIL_UTIL_H
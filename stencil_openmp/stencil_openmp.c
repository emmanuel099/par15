#include <stdlib.h>
#include <sys/time.h>

#include <omp.h>

#include <stencil/matrix.h>
#include <stencil/vector.h>
#include <stencil/util.h>

#include "stencil_openmp.h"

inline double stencil_five_point_kernel(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    return (stencil_matrix_get(matrix, row - 1, col) +
            stencil_matrix_get(matrix, row, col - 1) +
            stencil_matrix_get(matrix, row, col + 1) +
            stencil_matrix_get(matrix, row + 1, col)) * 0.25;
}

double five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix)
{
    assert(matrix->boundary >= 1);

    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, 0, 0, matrix->rows, matrix->cols, 0);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    #pragma omp parallel for schedule(static) shared(matrix, tmp_matrix)
    for (size_t row = matrix->boundary; row < rows; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(tmp_matrix, row, col);
            stencil_matrix_set(matrix, row, col, value);
        }
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    stencil_matrix_free(tmp_matrix);

    return time_difference_ms(t1, t2);
}

double five_point_stencil_with_one_vector(stencil_matrix_t *matrix)
{
    assert(matrix->boundary >= 1);

    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    #pragma omp parallel shared(matrix)
    {
        const int thread = omp_get_thread_num();
        const size_t rows_per_thread = (matrix->rows - 2 * matrix->boundary) / omp_get_num_threads();

        assert(((matrix->rows - 2 * matrix->boundary) % omp_get_num_threads()) == 0);

        const size_t start_row = thread * rows_per_thread + matrix->boundary;
        const size_t end_row = start_row + rows_per_thread - 1;

        stencil_vector_t *vec = stencil_vector_new(matrix->cols);
        stencil_vector_t *last_vec = stencil_vector_new(matrix->cols);

        // calculate the first and last row
        for (size_t col = matrix->boundary; col < cols; col++) {
            stencil_vector_set(vec, col, stencil_five_point_kernel(matrix, start_row, col));
            stencil_vector_set(last_vec, col, stencil_five_point_kernel(matrix, end_row, col));
        }

        // wait until all threads have filled the first and last row
        #pragma omp barrier

        // calculate the remaining rows
        for (size_t row = start_row + 1; row < end_row; row++) {
            for (size_t col = matrix->boundary; col < cols; col++) {
                const double value = stencil_five_point_kernel(matrix, row, col);
                // copy back the previosly calculated value before we overwrite it
                stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(vec, col));
                stencil_vector_set(vec, col, value);
            }
        }
        stencil_matrix_set_row(matrix, end_row - 1, vec);

        // copy back the last row
        stencil_matrix_set_row(matrix, end_row, last_vec);

        stencil_vector_free(vec);
        stencil_vector_free(last_vec);
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    return time_difference_ms(t1, t2);
}

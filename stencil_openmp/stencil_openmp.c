#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

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

double five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix, const size_t iterations)
{
    assert(matrix->boundary >= 1);

    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, 0, 0, matrix->rows, matrix->cols, matrix->boundary);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    const double t1 = omp_get_wtime();

    for (size_t iteration = 1; iteration <= iterations; iteration++) {
        #pragma omp parallel for schedule(static) shared(matrix, tmp_matrix)
        for (size_t row = matrix->boundary; row < rows; row++) {
            for (size_t col = matrix->boundary; col < cols; col++) {
                const double value = stencil_five_point_kernel(tmp_matrix, row, col);
                stencil_matrix_set(matrix, row, col, value);
            }
        }

        stencil_matrix_t *tmp = tmp_matrix;
        tmp_matrix = matrix;
        matrix = tmp;
    }

    const double t2 = omp_get_wtime();

    if (iterations % 2 != 0) {
        stencil_matrix_t *tmp = tmp_matrix;
        tmp_matrix = matrix;
        matrix = tmp;
    }

    stencil_matrix_free(tmp_matrix);

    return (t2 - t1) * 1000.0;
}

double five_point_stencil_with_one_vector(stencil_matrix_t *matrix, const size_t iterations)
{
    assert(matrix->boundary >= 1);

    const size_t cols = matrix->cols - matrix->boundary;

    double wall_time = 0.0;

    #pragma omp parallel shared(matrix) reduction(max : wall_time)
    {
        const int thread = omp_get_thread_num();
        const int threads = omp_get_num_threads();
        const bool is_last_thread = (thread == (threads - 1));
        const size_t rows_per_thread = (matrix->rows - 2 * matrix->boundary) / threads;

        const size_t start_row = thread * rows_per_thread + matrix->boundary;
        const size_t end_row = is_last_thread ? (matrix->rows - matrix->boundary - 1)
                                              : (start_row + rows_per_thread - 1);

        stencil_vector_t *vec = stencil_vector_new(matrix->cols);
        stencil_vector_t *last_vec = stencil_vector_new(matrix->cols);

        const double t1 = omp_get_wtime();

        for (size_t iteration = 1; iteration <= iterations; iteration++) {
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

            // wait for all threads before we start with the next iteration
            #pragma omp barrier
        }

        const double t2 = omp_get_wtime();

        stencil_vector_free(vec);
        stencil_vector_free(last_vec);

        wall_time = (t2 - t1) * 1000.0;
    }

    return wall_time;
}

double five_point_stencil_with_one_vector_tld(stencil_matrix_t *matrix, const size_t iterations)
{
    assert(matrix->boundary >= 1);

    double wall_time = 0.0;

    stencil_matrix_t **submatrices;

    #pragma omp parallel shared(matrix, submatrices) reduction(max : wall_time)
    {
        const int thread = omp_get_thread_num();
        const int threads = omp_get_num_threads();
        const bool is_first_thread = (thread == 0);
        const bool is_last_thread = (thread == (threads - 1));
        const size_t rows_per_thread = (matrix->rows - 2 * matrix->boundary) / threads;

        const size_t start_row = thread * rows_per_thread + matrix->boundary;
        const size_t end_row = is_last_thread ? (matrix->rows - matrix->boundary)
                                              : (start_row + rows_per_thread);

        stencil_matrix_t *submatrix = stencil_matrix_get_submatrix(matrix, start_row - 1,
                                                                   matrix->boundary - 1,
                                                                   end_row - start_row + 2,
                                                                   matrix->cols - matrix->boundary + 1, 1);
        stencil_vector_t *tmp = stencil_vector_new(submatrix->cols);

        // exchange matrix pointers with neighbouring threads
        #pragma omp single
        {
            submatrices = (stencil_matrix_t **)malloc(threads * sizeof(stencil_matrix_t *));
        }
        submatrices[thread] = submatrix;
        #pragma omp barrier
        stencil_matrix_t *submatrix_above = is_first_thread ? NULL : submatrices[thread - 1];
        stencil_matrix_t *submatrix_below = is_last_thread ? NULL : submatrices[thread + 1];

        const size_t rows = submatrix->rows - submatrix->boundary;
        const size_t cols = submatrix->cols - submatrix->boundary;

        const double t1 = omp_get_wtime();

        for (size_t iteration = 1; iteration <= iterations; iteration++) {
            // exchange boundary data (not needed on the first iteration because we
            // have already have the correct boundary data from the initial matrix)
            if (iteration > 1) {
                #pragma omp barrier

                if (submatrix_above != NULL) {
                    // exchange top
                    double *src = stencil_matrix_get_ptr(submatrix, 1, 0);
                    double *dest = stencil_matrix_get_ptr(submatrix_above, submatrix_above->rows - 1, 0);
                    memcpy(dest, src, submatrix->cols * sizeof(double));
                }
                if (submatrix_below != NULL) {
                    // exchange bottom
                    double *src = stencil_matrix_get_ptr(submatrix, submatrix->rows - 2, 0);
                    double *dest = stencil_matrix_get_ptr(submatrix_below, 0, 0);
                    memcpy(dest, src, submatrix->cols * sizeof(double));
                }

                // wait until all threads have exchanged their boundaries
                #pragma omp barrier
            }

            // calculate the first row
            const size_t first_row = submatrix->boundary;
            for (size_t col = submatrix->boundary; col < cols; col++) {
                stencil_vector_set(tmp, col, stencil_five_point_kernel(submatrix, first_row, col));
            }

            // calculate the remaining rows
            for (size_t row = first_row + 1; row < rows; row++) {
                for (size_t col = submatrix->boundary; col < cols; col++) {
                    const double value = stencil_five_point_kernel(submatrix, row, col);
                    // copy back the previosly calculated value before we overwrite it
                    stencil_matrix_set(submatrix, row - 1, col, stencil_vector_get(tmp, col));
                    stencil_vector_set(tmp, col, value);
                }
            }

            // copy back calculated values of the last non-boundary row
            stencil_matrix_set_row(submatrix, rows - 1, tmp);
        }

        const double t2 = omp_get_wtime();

        stencil_matrix_set_submatrix(matrix, start_row, matrix->boundary, submatrix);
        stencil_matrix_free(submatrix);

        stencil_vector_free(tmp);

        wall_time = (t2 - t1) * 1000.0;
    }

    free(submatrices);

    return wall_time;
}

double five_point_stencil_with_one_vector_columnwise(stencil_matrix_t *matrix, const size_t iterations)
{
    assert(matrix->boundary >= 1);

    const size_t rows = matrix->rows - matrix->boundary;

    double wall_time = 0.0;

    #pragma omp parallel shared(matrix) reduction(max : wall_time)
    {
        const int thread = omp_get_thread_num();
        const int threads = omp_get_num_threads();
        const bool is_last_thread = (thread == (threads - 1));
        const size_t cols_per_thread = (matrix->cols - 2 * matrix->boundary) / threads;

        const size_t start_col = thread * cols_per_thread + matrix->boundary;
        const size_t end_col = is_last_thread ? (matrix->cols - matrix->boundary - 1)
                                              : (start_col + cols_per_thread - 1);

        stencil_vector_t *vec = stencil_vector_new(matrix->rows);
        stencil_vector_t *last_vec = stencil_vector_new(matrix->rows);

        const double t1 = omp_get_wtime();

        for (size_t iteration = 1; iteration <= iterations; iteration++) {
            // calculate the first and last col
            for (size_t row = matrix->boundary; row < rows; row++) {
                stencil_vector_set(vec, row, stencil_five_point_kernel(matrix, row, start_col));
                stencil_vector_set(last_vec, row, stencil_five_point_kernel(matrix, row, end_col));
            }

            // wait until all threads have filled the first and last column
            #pragma omp barrier

            // calculate the remaining cols
            for (size_t col = start_col + 1; col < end_col; col++) {
                for (size_t row = matrix->boundary; row < rows; row++) {
                    const double value = stencil_five_point_kernel(matrix, row, col);
                    // copy back the previosly calculated value before we overwrite it
                    stencil_matrix_set(matrix, row, col - 1, stencil_vector_get(vec, row));
                    stencil_vector_set(vec, row, value);
                }
            }
            stencil_matrix_set_column(matrix, end_col - 1, vec);

            // copy back the last column
            stencil_matrix_set_column(matrix, end_col, last_vec);

            // wait for all threads before we start with the next iteration
            #pragma omp barrier
        }

        const double t2 = omp_get_wtime();

        stencil_vector_free(vec);
        stencil_vector_free(last_vec);

        wall_time = (t2 - t1) * 1000.0;
    }

    return wall_time;
}

inline double stencil_matrix_copy_column(stencil_matrix_t *src, stencil_matrix_t *dest, size_t src_col, size_t dest_col)
{
    assert(src_col >= 0 && src_col <= src->cols - 1);
    assert(dest_col >= 0 && dest_col <= dest->cols - 1);
    assert(src->rows == dest->rows);

    double *src_it = stencil_matrix_get_ptr(src, 0, src_col);
    double *src_end = stencil_matrix_get_ptr(src, src->rows, src_col);
    double *dest_it = stencil_matrix_get_ptr(dest, 0, dest_col);

    while(src_it != src_end) {
        *dest_it = *src_it;
        src_it += src->cols;
        dest_it += dest->cols;
    }
}

double five_point_stencil_with_one_vector_columnwise_tld(stencil_matrix_t *matrix, const size_t iterations)
{
    assert(matrix->boundary >= 1);

    double wall_time = 0.0;

    stencil_matrix_t **submatrices;

    #pragma omp parallel shared(matrix, submatrices) reduction(max : wall_time)
    {
        const int thread = omp_get_thread_num();
        const int threads = omp_get_num_threads();
        const bool is_first_thread = (thread == 0);
        const bool is_last_thread = (thread == (threads - 1));
        const size_t cols_per_thread = (matrix->cols - 2 * matrix->boundary) / threads;

        const size_t start_col= thread * cols_per_thread + matrix->boundary;
        const size_t end_col = is_last_thread ? (matrix->cols - matrix->boundary)
                                              : (start_col + cols_per_thread);

        stencil_matrix_t *submatrix = stencil_matrix_get_submatrix(matrix, matrix->boundary - 1,
                                                                   start_col - 1,
                                                                   matrix->rows - matrix->boundary + 1,
                                                                   end_col - start_col + 2, 1);
        stencil_vector_t *tmp = stencil_vector_new(submatrix->rows);

        // exchange matrix pointers with neighbouring threads
        #pragma omp single
        {
            submatrices = (stencil_matrix_t **)malloc(threads * sizeof(stencil_matrix_t *));
        }
        submatrices[thread] = submatrix;
        #pragma omp barrier
        stencil_matrix_t *submatrix_left = is_first_thread ? NULL : submatrices[thread - 1];
        stencil_matrix_t *submatrix_right = is_last_thread ? NULL : submatrices[thread + 1];

        const size_t rows = submatrix->rows - submatrix->boundary;
        const size_t cols = submatrix->cols - submatrix->boundary;

        const double t1 = omp_get_wtime();

        for (size_t iteration = 1; iteration <= iterations; iteration++) {
            // exchange boundary data (not needed on the first iteration because we
            // have already have the correct boundary data from the initial matrix)
            if (iteration > 1) {
                #pragma omp barrier

                if (submatrix_left != NULL) {
                    // exchange left column
                    stencil_matrix_copy_column(submatrix, submatrix_left, 1, submatrix_left->cols - 1);
                }
                if (submatrix_right != NULL) {
                    // exchange right column
                    stencil_matrix_copy_column(submatrix, submatrix_right, submatrix->cols - 2, 0);
                }

                // wait until all threads have exchanged their boundaries
                #pragma omp barrier
            }

            // calculate the first col
            const size_t first_col = submatrix->boundary;
            for (size_t row = submatrix->boundary; row < rows; row++) {
                stencil_vector_set(tmp, row, stencil_five_point_kernel(submatrix, row, first_col));
            }

            // calculate the remaining cols
            for (size_t col = first_col + 1; col < cols; col++) {
                for (size_t row = submatrix->boundary; row < rows; row++) {
                    const double value = stencil_five_point_kernel(submatrix, row, col);
                    // copy back the previosly calculated value before we overwrite it
                    stencil_matrix_set(submatrix, row, col - 1, stencil_vector_get(tmp, row));
                    stencil_vector_set(tmp, row, value);
                }
            }

            // copy back calculated values of the last non-boundary col
            stencil_matrix_set_column(submatrix, cols - 1, tmp);
        }

        const double t2 = omp_get_wtime();

        stencil_matrix_set_submatrix(matrix, matrix->boundary, start_col, submatrix);
        stencil_matrix_free(submatrix);

        stencil_vector_free(tmp);

        wall_time = (t2 - t1) * 1000.0;
    }

    free(submatrices);

    return wall_time;
}

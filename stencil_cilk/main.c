#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include <stencil/matrix.h>
#include <stencil/util.h>

inline double stencil_five_point_kernel(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    return (stencil_matrix_get(matrix, row - 1, col) +
            stencil_matrix_get(matrix, row, col - 1) +
            stencil_matrix_get(matrix, row, col + 1) +
            stencil_matrix_get(matrix, row + 1, col)) * 0.25;
}

static void five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix)
{
    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, 0, 0, matrix->rows, matrix->cols);

    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    for (size_t row = 1; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(tmp_matrix, row, col);
            stencil_matrix_set(matrix, row, col, value);
        }
    }

    stencil_matrix_free(tmp_matrix);
}

void five_point_stencil_with_one_vector(stencil_matrix_t *matrix)
{
    stencil_vector_t *tmp = stencil_matrix_get_row(matrix, 0);

    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    for (size_t row = 1; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }

    // copy back the last row
    stencil_matrix_set_row(matrix, rows - 1, tmp);

    stencil_vector_free(tmp);
}

static void five_point_stencil_with_two_vectors(stencil_matrix_t *matrix)
{
    stencil_vector_t *above = stencil_matrix_get_row(matrix, 0);
    stencil_vector_t *current = stencil_vector_new(matrix->cols);

    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    for (size_t row = 1; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            stencil_vector_set(current, col, value);
        }

        stencil_matrix_set_row(matrix, row - 1, above);
        stencil_vector_t *tmp = above;
        above = current;
        current = tmp;
    }

    // copy back calculated values of the last non-boundary row
    stencil_matrix_set_row(matrix, rows - 1, above);

    stencil_vector_free(above);
    stencil_vector_free(current);
}

void stencil_parallel(stencil_matrix_t *matrix, stencil_matrix_t **submatrices, void (*stencil_sequential)(stencil_matrix_t *matrix), size_t workers)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);

    /* calculate submatrices */
    for (size_t i = 0; i < workers; i++) {
        cilk_spawn stencil_sequential(submatrices[i]);
    }
    cilk_sync;

    /* copy values from submatrices back to main matrix */
    int matrix_row = 1;
    for (size_t i = 0; i < workers; i++) {
        for (size_t row = 1; row <= (submatrices[i]->rows - 2); ++row) {
            memcpy(stencil_matrix_get_ptr(matrix, matrix_row, 0), stencil_matrix_get_ptr(submatrices[i], row, 0), submatrices[i]->cols * sizeof(double));
        }
        ++matrix_row;
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    printf("elapsed time %fms\n", time_difference_ms(t1, t2));
}

stencil_vector_t* calc_first_row(stencil_matrix_t *matrix, size_t row)
{
    stencil_vector_t *vector = stencil_matrix_get_row(matrix, 0);

    for (size_t col = 1; col < matrix->cols; col++) {
        const double value = stencil_five_point_kernel(matrix, row, col);
        stencil_vector_set(vector, col, value);
    }

    return vector;
}

void update_values(stencil_matrix_t *matrix, size_t start_row, size_t end_row)
{
    stencil_vector_t *tmp = stencil_matrix_get_row(matrix, start_row - 1);

    const size_t rows = end_row - 1;
    const size_t cols = matrix->cols - 1;

    for (size_t row = start_row; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }

    // copy back the last row
    stencil_matrix_set_row(matrix, rows - 1, tmp);

    stencil_vector_free(tmp);
}

void five_point_stencil_with_one_vector_buffer_first_row(stencil_matrix_t *matrix, size_t workers, size_t rows_per_worker)
{
    stencil_vector_t **first_row_vectors = malloc(workers * sizeof(stencil_vector_t*));

    struct timeval t1;
    gettimeofday(&t1, NULL);

    for (size_t i = 0; i < workers; i++) {
        first_row_vectors[i] = cilk_spawn calc_first_row(matrix, i * rows_per_worker + 1);
    }
    cilk_sync;

    /* calculate other values */
    for (size_t i = 0; i < workers; i++) {
        size_t start_row = i * rows_per_worker;
        size_t end_row = start_row + rows_per_worker;
        cilk_spawn update_values(matrix, start_row + 2, end_row);
    }
    cilk_sync;

    /* copy first row vectors values back to matrix*/
    for (size_t i = 0; i < workers; i++) {
        stencil_matrix_set_row(matrix, i * rows_per_worker + 1, first_row_vectors[i]);
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    printf("elapsed time %fms\n", time_difference_ms(t1, t2));
}

int main(int argc, char **argv)
{
    const size_t rows = 10000 + 2;   // n + 2 boundary vectors
    const size_t cols = 20000 + 2;   // m + 2 boundary vectors
    const size_t workers = __cilkrts_get_nworkers();
    const size_t rows_per_worker = (rows - 2) / workers; // rows per worker without 2 overlapping rows

    // number of threads divides number of rows (without boundary)
    assert ((rows - 2) % workers == 0);

    stencil_matrix_t *matrix = new_randomized_matrix(rows, cols, 0, 1000);

    /* create submatrices */
    stencil_matrix_t **submatrices = malloc(workers * sizeof(stencil_matrix_t*));
    for (size_t i = 0; i < workers; i++) {
        submatrices[i] = stencil_matrix_get_submatrix(matrix, i * rows_per_worker, 0, rows_per_worker + 2, cols);
    }

    /* start computations */
    printf("five_point_stencil_with_one_vector_buffer_first_row: \n");
    for (int i = 0; i < 6; i++) {
        five_point_stencil_with_one_vector_buffer_first_row(matrix, workers, rows_per_worker);
    }

    /* start computations */
    printf("\nfive_point_stencil_with_one_vector: \n");
    for (int i = 0; i < 6; i++) {
        stencil_parallel(matrix, submatrices, five_point_stencil_with_one_vector, workers);
    }

    printf("\nfive_point_stencil_with_two_vectors: \n");
    for (int i = 0; i < 6; i++) {
        stencil_parallel(matrix, submatrices, five_point_stencil_with_two_vectors, workers);
    }

    printf("\nfive_point_stencil_with_tmp_matrix: \n");
    for (size_t i = 0; i < 6; i++) {
        stencil_parallel(matrix, submatrices, five_point_stencil_with_tmp_matrix, workers);
    }

    /* free memory */
    for (size_t i = 0; i < workers; i++) {
        stencil_matrix_free(submatrices[i]);
    }
    free(submatrices);
    stencil_matrix_free(matrix);

    return EXIT_SUCCESS;
}

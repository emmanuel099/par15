#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <stencil/util.h>
#include <stencil/matrix.h>
#include <stencil/vector.h>
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
    assert(matrix->boundary >= 1);

    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, 0, 0, matrix->rows, matrix->cols);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    for (size_t row = matrix->boundary; row < rows; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(tmp_matrix, row, col);
            stencil_matrix_set(matrix, row, col, value);
        }
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    printf("five_point_stencil_with_tmp_matrix - elapsed time %fms\n", time_difference_ms(t1, t2));

    stencil_matrix_free(tmp_matrix);
}

static void five_point_stencil_with_two_vectors(stencil_matrix_t *matrix)
{
    assert(matrix->boundary >= 1);

    stencil_vector_t *above = stencil_vector_new(matrix->cols);
    stencil_vector_t *current = stencil_vector_new(matrix->cols);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    // calculate the first row
    const size_t first_row = matrix->boundary;
    for (size_t col = matrix->boundary; col < cols; col++) {
        stencil_vector_set(above, col, stencil_five_point_kernel(matrix, first_row, col));
    }

    // calculate the remaining rows
    for (size_t row = matrix->boundary + 1; row < rows; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
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

    struct timeval t2;
    gettimeofday(&t2, NULL);

    printf("five_point_stencil_with_two_vectors - elapsed time %fms\n", time_difference_ms(t1, t2));

    stencil_vector_free(above);
    stencil_vector_free(current);
}

static void five_point_stencil_with_one_vector(stencil_matrix_t *matrix)
{
    assert(matrix->boundary >= 1);

    stencil_vector_t *tmp = stencil_vector_new(matrix->cols);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    // calculate the first row
    const size_t first_row = matrix->boundary;
    for (size_t col = matrix->boundary; col < cols; col++) {
        stencil_vector_set(tmp, col, stencil_five_point_kernel(matrix, first_row, col));
    }

    // calculate the remaining rows
    for (size_t row = matrix->boundary + 1; row < rows; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }

    // copy back calculated values of the last non-boundary row
    stencil_matrix_set_row(matrix, rows - 1, tmp);

    struct timeval t2;
    gettimeofday(&t2, NULL);

    printf("five_point_stencil_with_one_vector - elapsed time %fms\n", time_difference_ms(t1, t2));

    stencil_vector_free(tmp);
}

int main(int argc, char **argv)
{ 
    fprintf(stdout, "Hello sequential\n");

    stencil_matrix_t *matrix = stencil_matrix_new(10000, 20000, 1);
    for (int i = 0; i < 6; i++) {
        five_point_stencil_with_tmp_matrix(matrix);
        five_point_stencil_with_two_vectors(matrix);
        five_point_stencil_with_one_vector(matrix);
    }
    stencil_matrix_free(matrix);

    return EXIT_SUCCESS;
}

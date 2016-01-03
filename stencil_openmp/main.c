#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <omp.h>

#include <stencil/matrix.h>
#include <stencil/vector.h>

static double time_difference_ms(struct timeval t1, struct timeval t2)
{
    return (t2.tv_sec * 1000.0 + t2.tv_usec / 1000.0) - (t1.tv_sec * 1000.0 + t1.tv_usec / 1000.0);
}

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

    struct timeval t1;
    gettimeofday(&t1, NULL);

    #pragma omp parallel for schedule(static) firstprivate(rows, cols) shared(matrix, tmp_matrix)
    for (size_t row = 1; row < rows; row++) {
        for (size_t col = 1; col < cols; col++) {
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
    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    #pragma omp parallel firstprivate(rows, cols) shared(matrix)
    {
        stencil_vector_t *above = stencil_matrix_get_row(matrix, 0); // FIXME first block idx
        #pragma omp barrier
        stencil_vector_t *current = stencil_vector_new(matrix->cols);

        #pragma omp for schedule(static) nowait
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

        stencil_vector_free(above);
        stencil_vector_free(current);
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    printf("five_point_stencil_with_two_vectors - elapsed time %fms\n", time_difference_ms(t1, t2));
}

static void five_point_stencil_with_one_vector(stencil_matrix_t *matrix)
{
    const size_t rows = matrix->rows - 1;
    const size_t cols = matrix->cols - 1;

    struct timeval t1;
    gettimeofday(&t1, NULL);

    #pragma omp parallel firstprivate(rows, cols) shared(matrix)
    {
        stencil_vector_t *tmp = stencil_matrix_get_row(matrix, 0); // FIXME first block idx
        #pragma omp barrier

        #pragma omp for schedule(static) nowait
        for (size_t row = 1; row < rows; row++) {
            for (size_t col = 1; col < cols; col++) {
                const double value = stencil_five_point_kernel(matrix, row, col);
                // copy back the previosly calculated value before we overwrite it
                stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
                stencil_vector_set(tmp, col, value);
            }
        }

        stencil_vector_free(tmp);
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);

    printf("five_point_stencil_with_one_vector - elapsed time %fms\n", time_difference_ms(t1, t2));
}

int main(int argc, char **argv)
{ 
    fprintf(stdout, "Hello openmp\n");

    omp_set_num_threads(2);

    stencil_matrix_t *matrix = stencil_matrix_new(10000, 20000);
    for (int i = 0; i < 6; i++) {
        five_point_stencil_with_tmp_matrix(matrix);
        five_point_stencil_with_two_vectors(matrix);
        five_point_stencil_with_one_vector(matrix);
    }
    stencil_matrix_free(matrix);

    return EXIT_SUCCESS;
}

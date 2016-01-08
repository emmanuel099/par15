#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "stencil_mpi.h"

#define MASTER 0

enum mpi_tags_t {
    TAG_ROWS,
    TAG_COLS,
    TAG_DATA
};

inline double stencil_five_point_kernel(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    return (stencil_matrix_get(matrix, row - 1, col) +
            stencil_matrix_get(matrix, row, col - 1) +
            stencil_matrix_get(matrix, row, col + 1) +
            stencil_matrix_get(matrix, row + 1, col)) * 0.25;
}

static void sequential_five_point_stencil(stencil_matrix_t *matrix, const size_t iterations)
{
    assert(matrix->boundary >= 1);

    stencil_vector_t *tmp = stencil_vector_new(matrix->cols);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    for (size_t iteration = 1; iteration <= iterations; iteration++) {
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
    }

    stencil_vector_free(tmp);
}

static void send_matrix(const stencil_matrix_t* matrix, const size_t start_row, const size_t rows, const size_t recv)
{
    MPI_Send(&rows, 1, MPI_UNSIGNED_LONG, recv, TAG_ROWS, MPI_COMM_WORLD);
    MPI_Send(&matrix->cols, 1, MPI_UNSIGNED_LONG, recv, TAG_COLS, MPI_COMM_WORLD);

    // send submatrix
    MPI_Send(stencil_matrix_get_ptr(matrix, start_row, 0), matrix->cols * rows, MPI_DOUBLE, recv, TAG_DATA, MPI_COMM_WORLD);
}

int stencil_init(int *argc, char ***argv)
{
    return MPI_Init(argc, argv);
}

int stencil_finalize()
{
    return MPI_Finalize();
}

double five_point_stencil_host(stencil_matrix_t *matrix, size_t iterations)
{
    MPI_Bcast(&iterations, 1, MPI_UNSIGNED_LONG, MASTER, MPI_COMM_WORLD);

    int nr_workers = 1;
    MPI_Comm_size(MPI_COMM_WORLD, &nr_workers);

    const size_t rows = matrix->rows - 2 * matrix->boundary;
    const size_t rows_per_worker = rows / nr_workers;
    const size_t rows_for_last_worker = rows_per_worker + 2 + rows % nr_workers;

    // distribute submatrices
    for (size_t worker = 1; worker < (nr_workers - 1); worker++) {
        send_matrix(matrix, matrix->boundary + worker * rows_per_worker - 1, rows_per_worker + 2, worker);
    }
    // send to last worker
    const size_t start_row = matrix->boundary + (nr_workers - 1) * rows_per_worker - 1;
    send_matrix(matrix, start_row, rows_for_last_worker, nr_workers - 1);

    double t1 = MPI_Wtime();

    // calculate submatrix
    sequential_five_point_stencil(matrix, iterations);

    // receive submatrix data from other workers
    size_t matrix_row = matrix->boundary + rows_per_worker;
    for (size_t i = 1; i < (nr_workers - 1); i++) {
        MPI_Recv(stencil_matrix_get_ptr(matrix, matrix_row, 0), matrix->cols * rows_per_worker, MPI_DOUBLE, i, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        matrix_row += rows_per_worker;
    }
    // receive from last worker
    MPI_Recv(stencil_matrix_get_ptr(matrix, matrix_row, 0), matrix->cols * rows_for_last_worker, MPI_DOUBLE, (nr_workers - 1), TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    double t2 = MPI_Wtime();

    return (t2 - t1) * 1000;
}

void five_point_stencil_client(int rank)
{
    size_t iterations;
    MPI_Bcast(&iterations, 1, MPI_UNSIGNED_LONG, MASTER, MPI_COMM_WORLD);

    // receive matrix (with boundary)
    size_t rows, cols;
    MPI_Recv(&rows, 1, MPI_UNSIGNED_LONG, MASTER, TAG_ROWS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&cols, 1, MPI_UNSIGNED_LONG, MASTER, TAG_COLS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    const size_t boundary = 1;
    stencil_matrix_t *matrix = stencil_matrix_new(rows, cols, boundary);
    MPI_Recv(matrix->values, cols * rows, MPI_DOUBLE, MASTER, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // start calculation
    sequential_five_point_stencil(matrix, iterations);

    // send back data (without boundary)
    MPI_Datatype matrix_without_boundary;
    const int matrix_size[] = {rows, cols};
    const int data_size[] = {rows - 2 * boundary, cols - 2 * boundary};
    const int data_position[] = {boundary, boundary};
    MPI_Type_create_subarray(2, matrix_size, data_size, data_position, MPI_ORDER_C, MPI_DOUBLE, &matrix_without_boundary);
    MPI_Type_commit(&matrix_without_boundary);

    MPI_Send(matrix->values, 1, matrix_without_boundary, MASTER, TAG_DATA, MPI_COMM_WORLD);

    MPI_Type_free(&matrix_without_boundary);

    stencil_matrix_free(matrix);
}

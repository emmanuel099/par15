#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "stencil_mpi.h"

const int tag_rows = 0;
const int tag_cols = 1;
const int tag_boundary = 2;
const int tag_data = 3;

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

void send_matrix(const stencil_matrix_t* matrix, const size_t start_row, const size_t rows, const size_t recv)
{
    MPI_Send(&rows, 1, MPI_INT, recv, tag_rows, MPI_COMM_WORLD);
    MPI_Send(&matrix->cols, 1, MPI_INT, recv, tag_cols, MPI_COMM_WORLD);
    MPI_Send(&matrix->boundary, 1, MPI_INT, recv, tag_boundary, MPI_COMM_WORLD);

    // send submatrix
    MPI_Send(stencil_matrix_get_ptr(matrix, start_row, 0), matrix->cols * rows, MPI_DOUBLE, recv, tag_data, MPI_COMM_WORLD);
}

double five_point_stencil_host(stencil_matrix_t *matrix, const size_t iterations, size_t nr_workers)
{
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
    sequential_five_point_stencil(matrix, 1);

    // receive submatrix data from other workers
    size_t matrix_row = matrix->boundary + rows_per_worker;
    for (size_t i = 1; i < (nr_workers - 1); i++) {
        MPI_Recv(stencil_matrix_get_ptr(matrix, matrix_row, 0), matrix->cols * rows_per_worker, MPI_DOUBLE, i, tag_data, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        matrix_row += rows_per_worker;
    }
    // receive from last worker
    MPI_Recv(stencil_matrix_get_ptr(matrix, matrix_row, 0), matrix->cols * rows_for_last_worker, MPI_DOUBLE, (nr_workers - 1), tag_data, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    double t2 = MPI_Wtime();

    return (t2 - t1) * 1000;
}

void five_point_stencil_client(int rank)
{
    int rows, cols, boundary;
    // receive matrix
    MPI_Recv(&rows, 1, MPI_INT, 0, tag_rows, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&cols, 1, MPI_INT, 0, tag_cols, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&boundary, 1, MPI_INT, 0, tag_boundary, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    stencil_matrix_t *matrix = stencil_matrix_new(rows, cols, boundary);
    MPI_Recv(stencil_matrix_get_ptr(matrix, 0, 0), cols * rows, MPI_DOUBLE, 0, tag_data, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // start calculation
    sequential_five_point_stencil(matrix, 1);

    // send back data
    MPI_Send(stencil_matrix_get_ptr(matrix, 1, 0), (matrix->rows - 2) * matrix->cols, MPI_DOUBLE, 0, tag_data, MPI_COMM_WORLD);

    stencil_matrix_free(matrix);
}

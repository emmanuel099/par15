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

static stencil_vector_t* five_point_stencil_for_row(const stencil_matrix_t *matrix, const size_t row)
{
    stencil_vector_t *vector = stencil_vector_new(matrix->cols);
    const size_t cols = matrix->cols - matrix->boundary;

    for (size_t col = matrix->boundary; col < cols; col++) {
        stencil_vector_set(vector, col, stencil_five_point_kernel(matrix, row, col));
    }

    return vector;
}


static void five_point_stencil_with_tmp_matrix(stencil_matrix_t *matrix, const size_t start_row, const size_t rows)
{
    if (rows <= 0) {
        return;
    }

    const size_t end_row = start_row + rows;
    const size_t cols = matrix->cols - matrix->boundary;

    stencil_matrix_t *tmp_matrix = stencil_matrix_get_submatrix(matrix, start_row - 1, 0, rows + 2, matrix->cols, 0);

    for (size_t row = start_row; row < end_row; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            stencil_matrix_set(matrix, row, col, stencil_five_point_kernel(tmp_matrix, row - start_row + 1, col));
        }
    }

    stencil_matrix_free(tmp_matrix);
}

static void five_point_stencil_with_two_vectors(stencil_matrix_t *matrix, const size_t start_row, const size_t rows)
{
    if (rows <= 0) {
        return;
    }

    const size_t end_row = start_row + rows;
    const size_t cols = matrix->cols - matrix->boundary;

    // calculate the first row
    stencil_vector_t *above = five_point_stencil_for_row(matrix, start_row);
    stencil_vector_t *current = stencil_vector_new(matrix->cols);

    // calculate the remaining rows
    for (size_t row = start_row + 1; row < end_row; row++) {
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
    stencil_matrix_set_row(matrix, end_row - 1, above);

    stencil_vector_free(above);
    stencil_vector_free(current);
}

void five_point_stencil_with_one_vector(stencil_matrix_t *matrix, const size_t start_row, const size_t rows)
{
    if (rows <= 0) {
        return;
    }

    const size_t cols = matrix->cols - matrix->boundary;
    const size_t end_row = start_row + rows;

    // calculate first row
    stencil_vector_t *tmp = five_point_stencil_for_row(matrix, start_row);

    // calculate the remaining rows
    for (size_t row = start_row + 1; row < end_row; row++) {
        for (size_t col = matrix->boundary; col < cols; col++) {
            const double value = stencil_five_point_kernel(matrix, row, col);
            // copy back the previosly calculated value before we overwrite it
            stencil_matrix_set(matrix, row - 1, col, stencil_vector_get(tmp, col));
            stencil_vector_set(tmp, col, value);
        }
    }
    // copy back the last row
    stencil_matrix_set_row(matrix, end_row - 1, tmp);

    stencil_vector_free(tmp);
}

void send_matrix(const stencil_matrix_t* matrix, const size_t start_row, const size_t rows, const size_t recv)
{
    MPI_Send(&rows, 1, MPI_INT, recv, tag_rows, MPI_COMM_WORLD);
    MPI_Send(&matrix->cols, 1, MPI_INT, recv, tag_cols, MPI_COMM_WORLD);
    MPI_Send(&matrix->boundary, 1, MPI_INT, recv, tag_boundary, MPI_COMM_WORLD);

    // send submatrix
    for (int row = start_row; row < (start_row + rows); row++) {
        MPI_Send(stencil_matrix_get_ptr(matrix, row, 0), matrix->cols, MPI_DOUBLE, recv, tag_data + row - start_row, MPI_COMM_WORLD);
    }
}

double host(stencil_matrix_t *matrix, size_t nr_workers, void (*stencil_sequential)(stencil_matrix_t*, const size_t, const size_t))
{
    const size_t rows = matrix->rows - 2 * matrix->boundary;
    const size_t rows_per_worker = rows / nr_workers;

    // distribute submatrices
    for (size_t worker = 1; worker < (nr_workers - 1); worker++) {
        send_matrix(matrix, matrix->boundary + worker * rows_per_worker - 1, rows_per_worker + 2, worker);
    }
    // send to last worker
    const size_t start_row = matrix->boundary + (nr_workers - 1) * rows_per_worker - 1;
    send_matrix(matrix, start_row, rows_per_worker + 2 + rows % nr_workers, nr_workers - 1);

    double t1 = MPI_Wtime();

    // calculate submatrix
    stencil_sequential(matrix, matrix->boundary, rows_per_worker);

    // receive submatrix data from other workers
    size_t matrix_row = matrix->boundary + rows_per_worker;
    for (int i = 1; i < (nr_workers - 1); i++) {
        for (int row = 0; row < rows_per_worker; row++) {
            MPI_Recv(stencil_matrix_get_ptr(matrix, matrix_row, 0), matrix->cols, MPI_DOUBLE, i, tag_data + row, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            matrix_row++;
        }
    }
    // receive from last worker
    for (int row = 0; row < (rows_per_worker + rows % nr_workers); row++) {
        MPI_Recv(stencil_matrix_get_ptr(matrix, matrix_row, 0), matrix->cols, MPI_DOUBLE, (nr_workers - 1), tag_data + row, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        matrix_row++;
    }

    double t2 = MPI_Wtime();

    return (t2 - t1) * 1000;
}

void client(const size_t rank, void (*stencil_sequential)(stencil_matrix_t*, const size_t, const size_t))
{
    int rows, cols, boundary;
    // receive matrix
    MPI_Recv(&rows, 1, MPI_INT, 0, tag_rows, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&cols, 1, MPI_INT, 0, tag_cols, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&boundary, 1, MPI_INT, 0, tag_boundary, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    stencil_matrix_t *matrix = stencil_matrix_new(rows, cols, boundary);
    for (int row = 0; row < rows; row++) {
        MPI_Recv(stencil_matrix_get_ptr(matrix, row, 0), cols, MPI_DOUBLE, 0, tag_data + row, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    double *tmp = stencil_matrix_get_ptr(matrix, 0, 0);

    // start calculation
    stencil_sequential(matrix, 1, matrix->rows - 2);

    // send back data
    for (int row = 1; row < (rows - 1); row++) {
        MPI_Send(stencil_matrix_get_ptr(matrix, row, 0), matrix->cols, MPI_DOUBLE, 0, tag_data + row - 1, MPI_COMM_WORLD);
    }

    stencil_matrix_free(matrix);
}

double mpi_stencil_one_vector_host(stencil_matrix_t *matrix, const size_t nr_workers)
{
    return host(matrix, nr_workers, five_point_stencil_with_one_vector);
}

void mpi_stencil_one_vector_client(const size_t rank)
{
    client(rank, five_point_stencil_with_one_vector);
}

double mpi_stencil_two_vectors_host(stencil_matrix_t *matrix, const size_t nr_workers)
{
    return host(matrix, nr_workers, five_point_stencil_with_two_vectors);
}

void mpi_stencil_two_vectors_client(const size_t rank)
{
    client(rank, five_point_stencil_with_two_vectors);
}

double mpi_stencil_tmp_matrix_host(stencil_matrix_t *matrix, const size_t nr_workers)
{
    return host(matrix, nr_workers, five_point_stencil_with_tmp_matrix);
}

void mpi_stencil_tmp_matrix_client(const size_t rank)
{
    client(rank, five_point_stencil_with_tmp_matrix);
}



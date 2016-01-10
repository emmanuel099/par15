#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <alloca.h>

#include <stencil/vector.h>

#include "stencil_mpi.h"

#define MASTER 0
#define DIMENSIONS 2
#define STENCIL_BOUNDARY 1

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

int stencil_init(int *argc, char ***argv, MPI_Comm *comm_card)
{
    int ret;

    ret = MPI_Init(argc, argv);
    if (ret != MPI_SUCCESS) {
        return ret;
    }

    int nodes;
    ret = MPI_Comm_size(MPI_COMM_WORLD, &nodes);
    if (ret != MPI_SUCCESS) {
        return ret;
    }

    // determine a good grid size
    int dims[DIMENSIONS];
    memset(dims, 0, sizeof(int) * DIMENSIONS); // MPI_Dims_create only modifies 0 values
    ret = MPI_Dims_create(nodes, DIMENSIONS, dims);
    if (ret != MPI_SUCCESS) {
        return ret;
    }

    // create a non-periodic cartesian grid (allow reordering)
    int periods[DIMENSIONS];
    memset(periods, false, sizeof(int) * DIMENSIONS);
    ret = MPI_Cart_create(MPI_COMM_WORLD, DIMENSIONS, dims, periods, true, comm_card);
    if (ret != MPI_SUCCESS) {
        return ret;
    }

    return MPI_SUCCESS;
}

int stencil_finalize()
{
    return MPI_Finalize();
}

static MPI_Datatype create_submatrix_type(stencil_matrix_t *matrix,
                                          size_t rows, size_t cols, size_t boundary)
{
    const int matrix_size[] = {matrix->rows, matrix->cols};
    const int data_size[] = {rows, cols};
    const int data_position[] = {boundary, boundary};

    MPI_Datatype submatrix_type;
    MPI_Type_create_subarray(DIMENSIONS, matrix_size, data_size, data_position,
                             MPI_ORDER_C, MPI_DOUBLE, &submatrix_type);
    MPI_Type_commit(&submatrix_type);

    return submatrix_type;
}

static void five_point_stencil_node(stencil_matrix_t *matrix, size_t iterations, MPI_Comm comm_card)
{
    MPI_Bcast(&iterations, 1, MPI_UNSIGNED_LONG, MASTER, comm_card);
    MPI_Bcast(&matrix->rows, 1, MPI_UNSIGNED_LONG, MASTER, comm_card);
    MPI_Bcast(&matrix->cols, 1, MPI_UNSIGNED_LONG, MASTER, comm_card);
    MPI_Bcast(&matrix->boundary, 1, MPI_UNSIGNED_LONG, MASTER, comm_card);

    int nodes;
    MPI_Comm_size(comm_card, &nodes);

    int dims[DIMENSIONS];
    int periods[DIMENSIONS];
    int coords[DIMENSIONS];
    MPI_Cart_get(comm_card, DIMENSIONS, dims, periods, coords);

    const int nodes_horizontal = dims[0];
    const int nodes_vertical = dims[1];

    assert(((matrix->rows - 2 * matrix->boundary) % nodes_vertical) == 0);
    assert(((matrix->cols - 2 * matrix->boundary) % nodes_horizontal) == 0);

    const size_t rows_per_node = (matrix->rows - 2 * matrix->boundary) / nodes_vertical;
    const size_t cols_per_node = (matrix->cols - 2 * matrix->boundary) / nodes_horizontal;

    // datatype which represents a sub-matrix with boundary information
    MPI_Datatype matrix_with_boundary = create_submatrix_type(matrix,
                                                              rows_per_node + 2 * STENCIL_BOUNDARY,
                                                              cols_per_node + 2 * STENCIL_BOUNDARY,
                                                              0);

    // datatype which represents a sub-matrix without boundary information
    MPI_Datatype matrix_without_boundary = create_submatrix_type(matrix,
                                                                 rows_per_node,
                                                                 cols_per_node,
                                                                 STENCIL_BOUNDARY);

    // calculate sub-matrix displacements and block counts
    int *block_counts = (int *)alloca(nodes * sizeof(int));
    int *block_displacements = (int *)alloca(nodes * sizeof(int));
    for (int i = 0; i < nodes_vertical; i++) {
        for (int j = 0; j < nodes_horizontal; j++) {
            const int node = i * nodes_vertical + j;
            block_counts[node] = 1; // block count is always 1 because we use our special matrix type
            block_displacements[node] = matrix->boundary + j * cols_per_node - STENCIL_BOUNDARY +
            (matrix->boundary + i * rows_per_node - STENCIL_BOUNDARY) * matrix->cols;
        }
    }

    // receive matrix (with boundary)
    const size_t rows = rows_per_node + 2 * STENCIL_BOUNDARY;
    const size_t cols = cols_per_node + 2 * STENCIL_BOUNDARY;
    stencil_matrix_t *node_matrix = stencil_matrix_new(rows, cols, STENCIL_BOUNDARY);
    MPI_Scatterv(matrix->values, block_counts, block_displacements, matrix_with_boundary, // sender
                 node_matrix->values, rows * cols, MPI_DOUBLE, // receiver
                 MASTER, comm_card);

    // start calculation
    sequential_five_point_stencil(node_matrix, iterations);

    // send back data (without boundary)
    MPI_Datatype node_matrix_without_boundary = create_submatrix_type(node_matrix,
                                                                      rows_per_node,
                                                                      cols_per_node,
                                                                      STENCIL_BOUNDARY);
    MPI_Gatherv(node_matrix->values, 1, node_matrix_without_boundary, // sender
                matrix->values, block_counts, block_displacements, matrix_without_boundary, // receiver
                MASTER, comm_card);
    MPI_Type_free(&node_matrix_without_boundary);

    MPI_Type_free(&matrix_without_boundary);
    MPI_Type_free(&matrix_with_boundary);

    stencil_matrix_free(node_matrix);
}

double five_point_stencil_host(stencil_matrix_t *matrix, size_t iterations, MPI_Comm comm_card)
{
    assert(matrix->boundary == STENCIL_BOUNDARY);

    const double t1 = MPI_Wtime();

    five_point_stencil_node(matrix, iterations, comm_card);

    const double t2 = MPI_Wtime();

    return (t2 - t1) * 1000;
}

void five_point_stencil_client(MPI_Comm comm_card)
{
    stencil_matrix_t *matrix = stencil_matrix_new(0, 0, 0); // create a empty matrix (we don't need any memory for values)
    five_point_stencil_node(matrix, 0, comm_card);
    stencil_matrix_free(matrix);
}

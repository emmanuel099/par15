#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdarg.h>

#include <mpi.h>

#include <stencil/vector.h>

#include "stencil_mpi.h"

#define MASTER 0
#define DIMENSIONS 2
#define DIM_HORIZONTAL 0
#define DIM_VERTICAL 1
#define STENCIL_BOUNDARY 1

// for cart shifting
#define DIM_SHIFT_UP (-1)
#define DIM_SHIFT_DOWN 1
#define DIM_SHIFT_LEFT (-1)
#define DIM_SHIFT_RIGHT 1

#define NO_NEIGHBOUR (MPI_PROC_NULL)
#define NEIGHBOUR_ABOVE 0
#define NEIGHBOUR_BELOW 1
#define NEIGHBOUR_LEFT 2
#define NEIGHBOUR_RIGHT 3

enum mpi_tags_t {
    TOP_HALO_TAG,
    BOTTOM_HALO_TAG,
    LEFT_HALO_TAG,
    RIGHT_HALO_TAG
};

//#define SENDRECV_BOUNDARY_EXCHANGE
//#define NONBLOCKING_BOUNDARY_EXCHANGE
//#define ONESIDED_FENCE_BOUNDARY_EXCHANGE
//#define ONESIDED_PSCW_BOUNDARY_EXCHANGE

inline double stencil_five_point_kernel(const stencil_matrix_t *const matrix, size_t row, size_t col)
{
    return (stencil_matrix_get(matrix, row - 1, col) +
            stencil_matrix_get(matrix, row, col - 1) +
            stencil_matrix_get(matrix, row, col + 1) +
            stencil_matrix_get(matrix, row + 1, col)) * 0.25;
}

static void exchange_boundary_data_sendrecv(stencil_matrix_t *matrix,
                                            int neighbours_source[], int neighbours_dest[],
                                            MPI_Datatype matrix_row_t, MPI_Datatype matrix_col_t,
                                            MPI_Comm comm_card)
{
    MPI_Status status;

    MPI_Sendrecv(stencil_matrix_get_ptr(matrix, 1, 0), 1, matrix_row_t,
                 neighbours_dest[NEIGHBOUR_ABOVE], TOP_HALO_TAG,
                 stencil_matrix_get_ptr(matrix, matrix->rows - 1, 0), 1, matrix_row_t,
                 neighbours_source[NEIGHBOUR_BELOW], TOP_HALO_TAG, comm_card, &status);

    MPI_Sendrecv(stencil_matrix_get_ptr(matrix, matrix->rows - 2, 0), 1, matrix_row_t,
                 neighbours_dest[NEIGHBOUR_BELOW], BOTTOM_HALO_TAG,
                 stencil_matrix_get_ptr(matrix, 0, 0), 1, matrix_row_t,
                 neighbours_source[NEIGHBOUR_ABOVE], BOTTOM_HALO_TAG, comm_card, &status);

    MPI_Sendrecv(stencil_matrix_get_ptr(matrix, 0, 1), 1, matrix_col_t,
                 neighbours_dest[NEIGHBOUR_LEFT], LEFT_HALO_TAG,
                 stencil_matrix_get_ptr(matrix, 0, matrix->cols - 1), 1, matrix_col_t,
                 neighbours_source[NEIGHBOUR_RIGHT], LEFT_HALO_TAG, comm_card, &status);

    MPI_Sendrecv(stencil_matrix_get_ptr(matrix, 0, matrix->cols - 2), 1, matrix_col_t,
                 neighbours_dest[NEIGHBOUR_RIGHT], RIGHT_HALO_TAG,
                 stencil_matrix_get_ptr(matrix, 0, 0), 1, matrix_col_t,
                 neighbours_source[NEIGHBOUR_LEFT], RIGHT_HALO_TAG, comm_card, &status);
}

static void exchange_boundary_data_nonblocking(stencil_matrix_t *matrix,
                                               int neighbours_source[], int neighbours_dest[],
                                               MPI_Datatype matrix_row_t, MPI_Datatype matrix_col_t,
                                               MPI_Comm comm_card)
{
    int req_count = 0;
    MPI_Request reqs[8];
    MPI_Status states[8];

    if (neighbours_dest[NEIGHBOUR_ABOVE] != NO_NEIGHBOUR) {
        MPI_Isend(stencil_matrix_get_ptr(matrix, 1, 0), 1, matrix_row_t,
                  neighbours_dest[NEIGHBOUR_ABOVE], TOP_HALO_TAG, comm_card, &reqs[req_count++]);
        MPI_Irecv(stencil_matrix_get_ptr(matrix, 0, 0), 1, matrix_row_t,
                  neighbours_source[NEIGHBOUR_ABOVE], BOTTOM_HALO_TAG, comm_card, &reqs[req_count++]);
    }
    if (neighbours_dest[NEIGHBOUR_BELOW] != NO_NEIGHBOUR) {
        MPI_Isend(stencil_matrix_get_ptr(matrix, matrix->rows - 2, 0), 1, matrix_row_t,
                  neighbours_dest[NEIGHBOUR_BELOW], BOTTOM_HALO_TAG, comm_card, &reqs[req_count++]);
        MPI_Irecv(stencil_matrix_get_ptr(matrix, matrix->rows - 1, 0), 1, matrix_row_t,
                  neighbours_source[NEIGHBOUR_BELOW], TOP_HALO_TAG, comm_card, &reqs[req_count++]);
    }
    if (neighbours_dest[NEIGHBOUR_LEFT] != NO_NEIGHBOUR) {
        MPI_Isend(stencil_matrix_get_ptr(matrix, 0, 1), 1, matrix_col_t,
                  neighbours_dest[NEIGHBOUR_LEFT], LEFT_HALO_TAG, comm_card, &reqs[req_count++]);
        MPI_Irecv(stencil_matrix_get_ptr(matrix, 0, 0), 1, matrix_col_t,
                  neighbours_source[NEIGHBOUR_LEFT], RIGHT_HALO_TAG, comm_card, &reqs[req_count++]);
    }
    if (neighbours_dest[NEIGHBOUR_RIGHT] != NO_NEIGHBOUR) {
        MPI_Isend(stencil_matrix_get_ptr(matrix, 0, matrix->cols - 2), 1, matrix_col_t,
                  neighbours_dest[NEIGHBOUR_RIGHT], RIGHT_HALO_TAG, comm_card, &reqs[req_count++]);
        MPI_Irecv(stencil_matrix_get_ptr(matrix, 0, matrix->cols - 1), 1, matrix_col_t,
                  neighbours_source[NEIGHBOUR_RIGHT], LEFT_HALO_TAG, comm_card, &reqs[req_count++]);
    }

    MPI_Waitall(req_count, reqs, states);
}

static void exchange_boundary_data_onesided_fence(stencil_matrix_t *matrix,
                                                  int neighbours_source[], int neighbours_dest[],
                                                  MPI_Datatype matrix_row_t, MPI_Datatype matrix_col_t,
                                                  MPI_Win boundary_window, MPI_Comm comm_card)
{
    MPI_Win_fence(MPI_MODE_NOSTORE, boundary_window);

    if (neighbours_dest[NEIGHBOUR_ABOVE] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, 1, 0), 1, matrix_row_t, neighbours_dest[NEIGHBOUR_ABOVE],
                (matrix->rows - 1) * matrix->cols, 1, matrix_row_t, boundary_window);
    }
    if (neighbours_dest[NEIGHBOUR_BELOW] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, matrix->rows - 2, 0), 1, matrix_row_t, neighbours_dest[NEIGHBOUR_BELOW],
                0, 1, matrix_row_t, boundary_window);
    }
    if (neighbours_dest[NEIGHBOUR_LEFT] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, 0, 1), 1, matrix_col_t, neighbours_dest[NEIGHBOUR_LEFT],
                matrix->cols - 1, 1, matrix_col_t, boundary_window);
    }
    if (neighbours_dest[NEIGHBOUR_RIGHT] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, 0, matrix->cols - 2), 1, matrix_col_t, neighbours_dest[NEIGHBOUR_RIGHT],
                0, 1, matrix_col_t, boundary_window);
    }

    MPI_Win_fence(MPI_MODE_NOSUCCEED, boundary_window);
}

static void exchange_boundary_data_onesided_pscw(stencil_matrix_t *matrix,
                                                 int neighbours_source[], int neighbours_dest[],
                                                 MPI_Datatype matrix_row_t, MPI_Datatype matrix_col_t,
                                                 MPI_Win boundary_window, MPI_Group group,
                                                 MPI_Comm comm_card)
{
    MPI_Win_post(group, MPI_MODE_NOSTORE , boundary_window);
    MPI_Win_start(group, 0, boundary_window);

    if (neighbours_dest[NEIGHBOUR_ABOVE] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, 1, 0), 1, matrix_row_t, neighbours_dest[NEIGHBOUR_ABOVE],
                (matrix->rows - 1) * matrix->cols, 1, matrix_row_t, boundary_window);
    }
    if (neighbours_dest[NEIGHBOUR_BELOW] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, matrix->rows - 2, 0), 1, matrix_row_t, neighbours_dest[NEIGHBOUR_BELOW],
                0, 1, matrix_row_t, boundary_window);
    }
    if (neighbours_dest[NEIGHBOUR_LEFT] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, 0, 1), 1, matrix_col_t, neighbours_dest[NEIGHBOUR_LEFT],
                matrix->cols - 1, 1, matrix_col_t, boundary_window);
    }
    if (neighbours_dest[NEIGHBOUR_RIGHT] != NO_NEIGHBOUR) {
        MPI_Put(stencil_matrix_get_ptr(matrix, 0, matrix->cols - 2), 1, matrix_col_t, neighbours_dest[NEIGHBOUR_RIGHT],
                0, 1, matrix_col_t, boundary_window);
    }

    MPI_Win_complete(boundary_window);
    MPI_Win_wait(boundary_window);
}

static MPI_Group create_mpi_group(MPI_Group world_group, int size, ...)
{
    va_list args;
    va_start(args, size);

    int count = 0;
    int members[size];
    for (int i = 0; i < size; i++) {
        members[count] = va_arg(args, int);
        if (members[count] != NO_NEIGHBOUR) {
            ++count;
        }
    }

    va_end(args);

    MPI_Group group;
    MPI_Group_incl(world_group, count, members, &group);
    return group;
}

static double sequential_five_point_stencil(stencil_matrix_t *matrix, const size_t iterations, MPI_Comm comm_card)
{
    assert(matrix->boundary >= 1);

    const size_t rows = matrix->rows - matrix->boundary;
    const size_t cols = matrix->cols - matrix->boundary;

    // find our neighbours
    int neighbours_source[4];
    int neighbours_dest[4];
    MPI_Cart_shift(comm_card, DIM_HORIZONTAL, DIM_SHIFT_LEFT,
                   &neighbours_source[NEIGHBOUR_RIGHT], &neighbours_dest[NEIGHBOUR_LEFT]);
    MPI_Cart_shift(comm_card, DIM_HORIZONTAL, DIM_SHIFT_RIGHT,
                   &neighbours_source[NEIGHBOUR_LEFT], &neighbours_dest[NEIGHBOUR_RIGHT]);
    MPI_Cart_shift(comm_card, DIM_VERTICAL, DIM_SHIFT_UP,
                   &neighbours_source[NEIGHBOUR_BELOW], &neighbours_dest[NEIGHBOUR_ABOVE]);
    MPI_Cart_shift(comm_card, DIM_VERTICAL, DIM_SHIFT_DOWN,
                   &neighbours_source[NEIGHBOUR_ABOVE], &neighbours_dest[NEIGHBOUR_BELOW]);

#if (defined(ONESIDED_FENCE_BOUNDARY_EXCHANGE) || defined(ONESIDED_PSCW_BOUNDARY_EXCHANGE))
    MPI_Win boundary_window;
    MPI_Win_create(stencil_matrix_get_ptr(matrix, 0, 0), matrix->cols * matrix->rows * sizeof(double),
                   sizeof(double), MPI_INFO_NULL, comm_card, &boundary_window);
#if defined(ONESIDED_PSCW_BOUNDARY_EXCHANGE)
    MPI_Group world_group;
    MPI_Comm_group(comm_card, &world_group);

    MPI_Group group = create_mpi_group(world_group, 4, neighbours_dest[NEIGHBOUR_ABOVE],
                                                       neighbours_dest[NEIGHBOUR_BELOW],
                                                       neighbours_dest[NEIGHBOUR_LEFT],
                                                       neighbours_dest[NEIGHBOUR_RIGHT]);
#endif
#endif

    MPI_Datatype matrix_row_t;
    MPI_Type_vector(1, matrix->cols, matrix->cols, MPI_DOUBLE, &matrix_row_t);
    MPI_Type_commit(&matrix_row_t);

    MPI_Datatype matrix_col_t;
    MPI_Type_vector(matrix->rows, 1, matrix->cols, MPI_DOUBLE, &matrix_col_t);
    MPI_Type_commit(&matrix_col_t);

    stencil_vector_t *tmp = stencil_vector_new(matrix->cols);

    const double t1 = MPI_Wtime();

    for (size_t iteration = 1; iteration <= iterations; iteration++) {
        // exchange boundary data (not needed on the first iteration because we
        // have already received the correct boundary data from master)
        if (iteration > 1) {
            #if defined(SENDRECV_BOUNDARY_EXCHANGE)
                exchange_boundary_data_sendrecv(matrix, neighbours_source, neighbours_dest,
                                                matrix_row_t, matrix_col_t, comm_card);
            #elif defined(NONBLOCKING_BOUNDARY_EXCHANGE)
                exchange_boundary_data_nonblocking(matrix, neighbours_source, neighbours_dest,
                                                   matrix_row_t, matrix_col_t, comm_card);
            #elif defined(ONESIDED_FENCE_BOUNDARY_EXCHANGE)
                exchange_boundary_data_onesided_fence(matrix, neighbours_source, neighbours_dest,
                                                      matrix_row_t, matrix_col_t, boundary_window,
                                                      comm_card);
            #elif defined(ONESIDED_PSCW_BOUNDARY_EXCHANGE)
                exchange_boundary_data_onesided_pscw(matrix, neighbours_source, neighbours_dest,
                                                     matrix_row_t, matrix_col_t, boundary_window,
                                                     group, comm_card);
            #endif
        }

        // calculate the first row
        const size_t first_row = matrix->boundary;
        for (size_t col = matrix->boundary; col < cols; col++) {
            stencil_vector_set(tmp, col, stencil_five_point_kernel(matrix, first_row, col));
        }

        // calculate the remaining rows
        for (size_t row = first_row + 1; row < rows; row++) {
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

    const double t2 = MPI_Wtime();

    stencil_vector_free(tmp);

#if (defined(ONESIDED_FENCE_BOUNDARY_EXCHANGE) || defined(ONESIDED_PSCW_BOUNDARY_EXCHANGE))
    MPI_Win_free(&boundary_window);

#if defined(ONESIDED_PSCW_BOUNDARY_EXCHANGE)
    MPI_Group_free(&world_group);
    MPI_Group_free(&group);
#endif
#endif

    MPI_Type_free(&matrix_col_t);
    MPI_Type_free(&matrix_row_t);

    return (t2 - t1) * 1000;
}

static void optimize_dims_for_matrix(int dims[], stencil_matrix_t *matrix)
{
    if (((matrix->cols > matrix->rows) && (dims[DIM_HORIZONTAL] < dims[DIM_VERTICAL])) ||
        ((matrix->cols < matrix->rows) && (dims[DIM_HORIZONTAL] > dims[DIM_VERTICAL]))) { // transpose
        const int h = dims[DIM_HORIZONTAL];
        dims[DIM_HORIZONTAL] = dims[DIM_VERTICAL];
        dims[DIM_VERTICAL] = h;
    }

    const double matrix_ratio = (double)matrix->cols / (double)matrix->rows;
    const float initial_ratio = (float)dims[DIM_HORIZONTAL] / (float)dims[DIM_VERTICAL];

    if (initial_ratio > matrix_ratio) {
        while ((dims[DIM_HORIZONTAL] % 2) == 0) {
            const int h = dims[DIM_HORIZONTAL] / 2;
            const int v = dims[DIM_VERTICAL] * 2;
            if (((double)h / (double)v) >= matrix_ratio) {
                dims[DIM_HORIZONTAL] = h;
                dims[DIM_VERTICAL] = v;
            } else {
                break;
            }
        }
    } else {
        while ((dims[DIM_VERTICAL] % 2) == 0) {
            const int h = dims[DIM_HORIZONTAL] * 2;
            const int v = dims[DIM_VERTICAL] / 2;
            if (((double)h / (double)v) <= matrix_ratio) {
                dims[DIM_HORIZONTAL] = h;
                dims[DIM_VERTICAL] = v;
            } else {
                break;
            }
        }
    }
}

static MPI_Comm create_cartesian_topology(MPI_Comm old_comm, stencil_matrix_t *matrix)
{
    int nodes;
    MPI_Comm_size(old_comm, &nodes);

    // determine a good grid size
    int dims[DIMENSIONS];
    memset(dims, 0, sizeof(int) * DIMENSIONS); // MPI_Dims_create only modifies 0 values
    MPI_Dims_create(nodes, DIMENSIONS, dims);
    optimize_dims_for_matrix(dims, matrix);

    // create a non-periodic cartesian grid (allow reordering)
    MPI_Comm comm_card;
    int periods[DIMENSIONS];
    memset(periods, 0, sizeof(int) * DIMENSIONS);
    MPI_Cart_create(old_comm, DIMENSIONS, dims, periods, true, &comm_card);

    return comm_card;
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

    MPI_Datatype resized_submatrix_type;
    MPI_Type_create_resized(submatrix_type, 0, sizeof(double), &resized_submatrix_type);
    MPI_Type_commit(&resized_submatrix_type);

    return resized_submatrix_type;
}

static double five_point_stencil_node(stencil_matrix_t *matrix, size_t iterations)
{
    MPI_Bcast(&iterations, 1, MPI_UNSIGNED_LONG, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&matrix->rows, 1, MPI_UNSIGNED_LONG, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&matrix->cols, 1, MPI_UNSIGNED_LONG, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&matrix->boundary, 1, MPI_UNSIGNED_LONG, MASTER, MPI_COMM_WORLD);

    MPI_Comm comm_card = create_cartesian_topology(MPI_COMM_WORLD, matrix);

    int nodes;
    MPI_Comm_size(comm_card, &nodes);

    int dims[DIMENSIONS];
    int periods[DIMENSIONS];
    int coords[DIMENSIONS];
    MPI_Cart_get(comm_card, DIMENSIONS, dims, periods, coords);

    const int nodes_horizontal = dims[DIM_HORIZONTAL];
    const int nodes_vertical = dims[DIM_VERTICAL];

    assert(((matrix->rows - 2 * matrix->boundary) % nodes_vertical) == 0);
    assert(((matrix->cols - 2 * matrix->boundary) % nodes_horizontal) == 0);

    const size_t rows_per_node = (matrix->rows - 2 * matrix->boundary) / nodes_vertical;
    const size_t cols_per_node = (matrix->cols - 2 * matrix->boundary) / nodes_horizontal;

    // calculate sub-matrix displacements and block counts (grid looks like [[0,2],[1,3]])
    int block_counts[nodes];
    int block_displacements[nodes];
    for (int i = 0; i < nodes_horizontal; i++) {
        for (int j = 0; j < nodes_vertical; j++) {
            const int node = i * nodes_vertical + j;
            block_counts[node] = 1; // block count is always 1 because we use our special matrix type
            block_displacements[node] = stencil_matrix_get_ptr(matrix, j * rows_per_node, i * cols_per_node) -
                                        stencil_matrix_get_ptr(matrix, 0, 0); // we need the relative address
        }
    }

    // receive matrix (with boundary)
    const size_t rows_per_node_with_boundary = rows_per_node + 2 * STENCIL_BOUNDARY;
    const size_t cols_per_node_with_boundary = cols_per_node + 2 * STENCIL_BOUNDARY;
    stencil_matrix_t *node_matrix = stencil_matrix_new(rows_per_node_with_boundary,
                                                       cols_per_node_with_boundary,
                                                       STENCIL_BOUNDARY);

    MPI_Datatype matrix_with_boundary_t = create_submatrix_type(matrix,
                                                                rows_per_node_with_boundary,
                                                                cols_per_node_with_boundary,
                                                                0);
    MPI_Datatype node_matrix_with_boundary_t = create_submatrix_type(node_matrix,
                                                                     rows_per_node_with_boundary,
                                                                     cols_per_node_with_boundary,
                                                                     0);

    MPI_Scatterv(matrix->values, block_counts, block_displacements, matrix_with_boundary_t, // sender
                 node_matrix->values, 1, node_matrix_with_boundary_t, // receiver
                 MASTER, comm_card);

    MPI_Type_free(&node_matrix_with_boundary_t);
    MPI_Type_free(&matrix_with_boundary_t);

    // start calculation
    double wall_time = sequential_five_point_stencil(node_matrix, iterations, comm_card);

    // send back data (without boundary)
    MPI_Datatype matrix_without_boundary_t = create_submatrix_type(matrix,
                                                                   rows_per_node,
                                                                   cols_per_node,
                                                                   STENCIL_BOUNDARY);
    MPI_Datatype node_matrix_without_boundary_t = create_submatrix_type(node_matrix,
                                                                        rows_per_node,
                                                                        cols_per_node,
                                                                        STENCIL_BOUNDARY);

    MPI_Gatherv(node_matrix->values, 1, node_matrix_without_boundary_t, // sender
                matrix->values, block_counts, block_displacements, matrix_without_boundary_t, // receiver
                MASTER, comm_card);

    MPI_Type_free(&node_matrix_without_boundary_t);
    MPI_Type_free(&matrix_without_boundary_t);

    stencil_matrix_free(node_matrix);

    // collect the maximum wall time
    double max_wall_time;
    MPI_Reduce(&wall_time, &max_wall_time, 1, MPI_DOUBLE, MPI_MAX, MASTER, comm_card);

    MPI_Comm_free(&comm_card);

    return max_wall_time;
}

double five_point_stencil_host(stencil_matrix_t *matrix, size_t iterations)
{
    assert(matrix->boundary == STENCIL_BOUNDARY);

    return five_point_stencil_node(matrix, iterations);
}

void five_point_stencil_client()
{
    stencil_matrix_t *matrix = stencil_matrix_new(0, 0, 0); // create a empty matrix (we don't need any memory for values)
    five_point_stencil_node(matrix, 0);
    stencil_matrix_free(matrix);
}

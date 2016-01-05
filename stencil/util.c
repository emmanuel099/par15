#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "util.h"

stencil_matrix_t* new_matrix_from_file(const char* filepath)
{
    FILE *stream = fopen(filepath, "r");
    if (stream == NULL) {
        return NULL;
    }

    /* read number of rows, columns and boundary size */
    char* line = NULL;
    size_t len = 0;
    if (getline(&line, &len, stream) == 0) {
        goto exit;
    }

    char* rows_string = strtok(line, ";");
    char* cols_string = strtok(NULL, ";");
    char* boundary_string = strtok(NULL, ";");

    if (rows_string == NULL || cols_string == NULL || boundary_string == NULL) {
        goto exit;
    }
    
    size_t rows = strtol(rows_string, NULL, 10);
    size_t cols = strtol(cols_string, NULL, 10);
    size_t boundary = strtol(boundary_string, NULL, 10);

    if (boundary < 1 || rows < 2 || cols < 2) { // 2 because of 2 * boundary
        goto exit;
    }

    /* create matrix */
    stencil_matrix_t* matrix = stencil_matrix_new(rows, cols, boundary);
    if (matrix == NULL) {
        goto exit;
    }

    /* read matrix values */
    matrix->boundary = 0; // we need to be able to change the boundary values
    for (size_t row = 0; row < matrix->rows; ++row) {
        if (getline(&line, &len, stream) == 0) {
            goto exit_matrix;
        }

        char* record = strtok(line, ";");
        for (size_t col = 0; col < matrix->cols; ++col) {
            if (record == NULL) {
                goto exit_matrix;
            }
            stencil_matrix_set(matrix, row, col, strtod(record, NULL));
            record = strtok(NULL, ";");
        }
    }
    matrix->boundary = boundary; // change it back to the original value

    fclose(stream);
    return matrix;

exit_matrix:
    free(matrix);
exit:
    fclose(stream);
    return NULL;
}

bool matrix_to_file(const stencil_matrix_t* matrix, FILE *stream)
{
    if (stream == NULL) {
        return false;
    }

    for(size_t row = 0; row < matrix->rows; ++row) {
        for(size_t col = 0; col < matrix->cols; ++col) {
           if (fprintf(stream, "%.3f;", stencil_matrix_get(matrix, row, col)) < 0) {
               return false;
           }
        }
        if (fprintf(stream, "\n") < 0) {
            return false;
        }
    }
    return true;
}

stencil_matrix_t* new_randomized_matrix(size_t rows, size_t cols, size_t boundary, int min_value, int max_value)
{
    stencil_matrix_t* matrix = stencil_matrix_new(rows, cols, boundary);
    if (matrix == NULL) {
        return NULL;
    }

    srand((unsigned int)time(NULL));

    matrix->boundary = 0; // we need to be able to change the boundary values
    for (size_t row = 0; row < matrix->rows; ++row) {
        for (size_t col = 0; col < matrix->cols; ++col) {
            stencil_matrix_set(matrix, row, col, min_value + rand() % max_value);
        }
    }
    matrix->boundary = boundary; // change it back to the original value

    return matrix;
}

double time_difference_ms(struct timeval t1, struct timeval t2)
{
    return (t2.tv_sec * 1000.0 + t2.tv_usec / 1000.0) - (t1.tv_sec * 1000.0 + t1.tv_usec / 1000.0);
}

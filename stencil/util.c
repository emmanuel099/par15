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

    /* read number of rows and columns */
    char* line = NULL;
    size_t len = 0;
    if (getline(&line, &len, stream) == 0) {
        goto exit;
    }

    char* rows_string = strtok(line, ";");
    char* cols_string = strtok(NULL, ";");

    if (rows_string == NULL || cols_string == NULL) {
        goto exit;
    }
    
    size_t rows = strtol(rows_string, NULL, 10);
    size_t cols = strtol(cols_string, NULL, 10);

    if (rows <= 0 || cols <= 0) {
        goto exit;
    }

    /* create matrix */
    stencil_matrix_t* matrix = stencil_matrix_new(rows, cols);
    if (matrix == NULL) {
        goto exit;
    }

    /* read matrix values */
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

    fclose(stream);
    return matrix;

exit_matrix:
    free(matrix);
exit:
    fclose(stream);
    return NULL;
}

bool matrix_to_file(const stencil_matrix_t* matrix, const char* filepath)
{
    FILE *stream = fopen(filepath, "w");
    if (stream == NULL) {
        return false;
    }

    for(size_t row = 0; row < matrix->rows; ++row) {
        for(size_t col = 0; col < matrix->cols; ++col) {
           if (fprintf(stream, "%.3f;", stencil_matrix_get(matrix, row, col)) < 0) {
               fclose(stream);
               return false;
           }
        }
        if (fprintf(stream, "\n") < 0) {
            fclose(stream);
            return false;
        }
    }

    fclose(stream);
    return true;
}

stencil_matrix_t* new_randomized_matrix(int rows, int cols, int min_value, int max_value)
{
    stencil_matrix_t* matrix = stencil_matrix_new(rows, cols);
    if (matrix == NULL) {
        return NULL;
    }

    srand((unsigned int)time(NULL));

    for (size_t row = 0; row < matrix->rows; ++row) {
        for (size_t col = 0; col < matrix->cols; ++col) {
            stencil_matrix_set(matrix, row, col, min_value + rand() % max_value);
        }
    }

    return matrix;
}

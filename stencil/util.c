#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

stencil_matrix_t* matrix_from_file(const char* filepath)
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

    /* read matrix values */
    for (int row = 0; row < matrix->rows; ++row) {
        if (getline(&line, &len, stream) == 0) {
            goto exit_matrix;
        }

        char* record = strtok(line, ";");
        for (int col = 0; col < matrix->cols; ++col) {
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
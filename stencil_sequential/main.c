#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <stencil/matrix.h>

/**
 * program argument structure
 */
struct args {
    ssize_t rows;
    ssize_t cols;
    char* input_file;
    char* output_file;
};
typedef struct args args_t;

/**
 * parses the program arguments and stores the argumets in \a args
 *
 * @param argc argument count
 * @param argv argument pointer
 * @param args read arguments
 *
 * @return returns -1 on error
 */
int process_arguments(int argc, char**argv, args_t *args);

/**
 * initializes the matrix values from a provided csv file
 *
 * @param matrix matrix to fill with values
 * @param input path to csv file
 *
 * @return returns -1 on error
 */
int init_matrix_from_file(stencil_matrix_t *matrix, char* input);

/**
 * initializes the matrix \a matrix with random values in the range from \a min to \a max
 *
 * @param min minimum number
 * @param max maximum number
 *
 */
void init_matrix_random(stencil_matrix_t *matrix, int min, int max);

/**
 * writes the matrix to a csv file
 *
 * @param matrix matrix to write
 * @param output csv file path
 *
 * @return returns -1 on error
 */
int matrix_to_file(stencil_matrix_t *matrix, char* output);

int main(int argc, char **argv)
{
    fprintf(stdout, "Hello sequential\n");

    args_t args = {0,0, NULL, NULL};
    if (process_arguments(argc, argv, &args) != 0) {
        fprintf(stdout, "ERROR: parsing arguments failed\n");
        fprintf(stdout, "arguments: \n -i <input file> \n -o <output file> optional \n -r <rows> \n -c <columns> ");
        return EXIT_FAILURE;
    }

    assert(args.rows > 0);
    assert(args.cols > 0);

    // create matrix
    stencil_matrix_t *matrix = stencil_matrix_new(args.rows, args.cols);

    if (args.input_file == NULL) {
        init_matrix_random(matrix, 0, 100000);
    } else if (init_matrix_from_file(matrix, args.input_file) != 0) {
        stencil_matrix_free(matrix);
        return EXIT_FAILURE;
    }

    // iterators
    stencil_matrix_copy_row_iterator_t *prev_row_copy_it = stencil_matrix_copy_row_iterator_new(matrix);
    stencil_matrix_row_iterator_next(prev_row_copy_it);
    stencil_matrix_copy_row_iterator_t *current_row_copy_it = stencil_matrix_copy_row_iterator_new(matrix);
    stencil_matrix_row_iterator_skip(current_row_copy_it, 1);

    stencil_matrix_mutable_row_iterator_t *current_row_it = stencil_matrix_mutable_row_iterator_new(matrix);
    stencil_matrix_mutable_row_iterator_t *next_row_it = stencil_matrix_mutable_row_iterator_new(matrix);
    stencil_matrix_row_iterator_skip(current_row_it, 1);
    stencil_matrix_row_iterator_skip(next_row_it, 2);

    struct timeval t1;
    gettimeofday(&t1, NULL);

    while (stencil_matrix_row_iterator_next(next_row_it)) {
        stencil_matrix_row_iterator_next(current_row_copy_it);
        stencil_matrix_row_iterator_next(current_row_it);

        for (int col = 1; col < (matrix->cols - 1); col++) {
            const double value = stencil_matrix_row_get(prev_row_copy_it, col) +
                                 stencil_matrix_row_get(current_row_copy_it, col - 1) +
                                 stencil_matrix_row_get(current_row_copy_it, col + 1) +
                                 stencil_matrix_row_get(next_row_it, col);
            stencil_matrix_row_set(current_row_it, col, value / 4.0);
        }

        stencil_matrix_copy_row_iterator_t *tmp = current_row_copy_it;
        current_row_copy_it = prev_row_copy_it;
        prev_row_copy_it = tmp;

        stencil_matrix_row_iterator_skip(current_row_copy_it, 1);
    }

    struct timeval t2;
    gettimeofday(&t2, NULL);
    printf("matrix - elapsed time %fms\n", (t2.tv_sec * 1000.0 + t2.tv_usec / 1000.0) - (t1.tv_sec * 1000.0 + t1.tv_usec / 1000.0));

    // print
    if (args.output_file != NULL) {
        matrix_to_file(matrix, args.output_file);
    }

    stencil_matrix_row_iterator_free(prev_row_copy_it);
    stencil_matrix_row_iterator_free(current_row_copy_it);
    stencil_matrix_row_iterator_free(current_row_it);
    stencil_matrix_row_iterator_free(next_row_it);

    stencil_matrix_free(matrix);
    return EXIT_SUCCESS;
}

int process_arguments(int argc, char**argv, args_t *args)
{
    char getopt_res;
    int flag_r = 0;
    int flag_c = 0;

    while ((getopt_res = getopt(argc, argv, "i:o:r:c:")) != -1) {
        switch (getopt_res) {
            case 'i': // input file
                args->input_file = optarg;
                break;
            case 'o': // output file
                args->output_file = optarg;
                break;
            case 'r': // rows
                flag_r = 1;
                args->rows = strtol(optarg, NULL, 10);
                break;
            case 'c': // cols
                flag_c = 1;
                args->cols = strtol(optarg, NULL, 10);
                break;
            default:
                return -1;
        }
    }

    if (flag_c == 0 || flag_r == 0) {
        return -1;
    }

    return 0;
}

int init_matrix_from_file(stencil_matrix_t *matrix, char* in)
{
    FILE *stream = fopen(in, "r");
    if (stream == NULL) {
        return -1;
    }

    char* line = NULL;
    char *record = NULL;
    size_t len = 0;

    for (int row = 0; row < matrix->rows; ++row) {
        if (getline(&line, &len, stream) == 0) {
            fclose(stream);
            return -1;
        }

        record = strtok(line, ";");
        for (int col = 0; col < matrix->cols; ++col) {
            stencil_matrix_set(matrix, row, col, strtod(record, NULL));
            record = strtok(NULL, ";");
        }
    }

    fclose(stream);
    return 0;
}

void init_matrix_random(stencil_matrix_t *matrix, int min, int max)
{
    srand((unsigned int)time(NULL));

    for (int row = 0; row < matrix->rows; ++row) {
        for (int col = 0; col < matrix->cols; ++col) {
            stencil_matrix_set(matrix, row, col, min + rand() % max);
        }
    }
}

int matrix_to_file(stencil_matrix_t *matrix, char* out)
{
    FILE *stream = fopen(out, "w");
    if (stream == NULL) {
        return -1;
    }

    for(int row = 1; row < (matrix->rows - 1); ++row) {
        for(int col = 1; col < (matrix->cols - 1); ++col) {
           fprintf(stream, "%f;", stencil_matrix_get(matrix, row, col));
        }
        fprintf(stream, "\n");
    }

    fclose(stream);
    return 0;
}
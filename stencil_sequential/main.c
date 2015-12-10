#include <stdio.h>
#include <stdlib.h>

#include <stencil/matrix.h>

int main(int argc, char **argv)
{ 
    fprintf(stdout, "Hello sequential\n");

    stencil_matrix_t *matrix = stencil_matrix_new(3, 4);
    stencil_matrix_free(matrix);

    return EXIT_SUCCESS;
}

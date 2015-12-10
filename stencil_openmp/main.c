#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#include <stencil/matrix.h>

int main(int argc, char **argv)
{ 
    stencil_matrix_t *matrix = stencil_matrix_new(3, 4);
    stencil_matrix_free(matrix);

    omp_set_num_threads(2);

    #pragma omp parallel num_threads(2)
    {
        fprintf(stdout, "Hello OpenMP\n");
    }

    return EXIT_SUCCESS;
}

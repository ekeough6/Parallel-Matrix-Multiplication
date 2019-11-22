#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include "matrix.h"
#include "ring.h"

int main(int argc,  char** argv) {
  MPI_Init(NULL, NULL);
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int size = 24;

  if(world_rank == 0) {

    time_t t;
    srand((unsigned) time(&t));
    float* iden = malloc(size * size * sizeof(float));

    int i, j;
    for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        iden[i * size + j] = (i == j) ? 1 : 0;
      }
    }

    printf("MAT \n");
    float* mat = generate_matrix(size, size);
    for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        printf("%6.3f ", mat[i * size + j]);
      }
      printf("\n");
    }
    printf("\n");

    printf("MAT 1\n");
    float* mat1 = generate_matrix(size, size);
    for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        printf("%6.3f ", mat1[i * size + j]);
      }
      printf("\n");
    }
    printf("\n");

    float* resultant = matrix_mult(mat, size, size, mat1, size, size);
    for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        printf("%6.3f ", resultant[i * size + j]);
      }
      printf("\n");
    }
    printf("\n");
    free(resultant);

    resultant = ring_mult(mat, size, size, mat1, size, size);
    for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        printf("%6.3f ", resultant[i * size + j]);
      }
      printf("\n");
    }
    printf("\n");
    free(iden);
    free(mat);
    free(resultant);
  } else {
    ring_mult_helper(size, size, size, size);
  }
  MPI_Finalize();
}

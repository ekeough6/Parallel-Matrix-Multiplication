#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "matrix.h"
#include "bmr.h"

int main(int argc,  char** argv) {
  MPI_Init(NULL, NULL);
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int size = 6 * 4;

  if(world_rank == 0) {
    float* iden = malloc(size * size * sizeof(float));
    //mmalloc((void**) &iden, size * size * sizeof(float));

    int i, j;
    for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        iden[i * size + j] = (i == j) ? 1 : 0;
      }
    }

    float* mat = generate_matrix(size, size);
    for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        printf("%6.3f ", mat[i * size + j]);
      }
      printf("\n");
    }
    printf("\n");

    float* mat1 = generate_matrix(size, size);
    /*for(i = 0; i < size; ++i) {
      for(j = 0; j < size; ++j) {
        printf("%6.3f ", mat1[i * size + j]);
      }
      printf("\n");
    }
    printf("\n");*/


    float* resultant = bmr_mult(mat, size, size, iden, size, size);
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
    bmr_mult_helper(size, size, size, size);
  }
  MPI_Finalize();
}

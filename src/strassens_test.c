#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "matrix.h"
#include "strassens.h"

int main(int argc,  char** argv) {
  MPI_Init(NULL, NULL);
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int sizes[3] = {(1<<8), (1<<10), (1<<12)};
  char* input[3] = {"sinput0.txt", "sinput1.txt", "sinput2.txt"};
  char* input2[3] = {"sinput00.txt", "sinput11.txt", "sinput22.txt"};
  char* output[3] = {"strassens_output0.txt", "strassens_output1.txt", "strassens_output2.txt"};
  int i, j, k;

  for(i = 0; i < 1; ++i) {
      if(world_rank == 0) {
        int r, c;

        float* mat = load_matrix(input[i], &r, &c);
        float* mat1 = load_matrix(input2[i], &r, &c);
        double time = MPI_Wtime();
        //printf("%6.3f\n", mat[0]);
        float* resultant = strassen_mult(mat, mat1, sizes[i], 0, 0);

        printf("2^%d*24: %f\n", i, MPI_Wtime() - time);
        //save_matrix(resultant, sizes[i], sizes[i], output[i]);

        FREE(mat);
        FREE(resultant);
        FREE(mat1);
      } else {
        strassen_mult_helper(sizes[i], 0, 0);
      }
  }
  MPI_Finalize();
}

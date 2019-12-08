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

  int sizes[3] = {(1<<4) * 24, (1<<6) * 24, (1<<8) * 24};
  char* input[3] = {"input0.txt", "input1.txt", "input2.txt"};
  char* input2[3] = {"input00.txt", "input11.txt", "input22.txt"};
  char* output[3] = {"ring_output0.txt", "ring_output1.txt", "ring_output2.txt"};
  int i, j, k;
  for(i = 0; i < 3; ++i) {
      if(world_rank == 0) {
        int r,c;

        float* mat = load_matrix(input[i], &r, &c);
        float* mat1 = load_matrix(input2[i], &r, &c);
        double time = MPI_Wtime();
        float* resultant = ring_mult(mat, sizes[i], sizes[i], mat1, sizes[i], sizes[i]);

        printf("2^%d*24: %f\n", i, MPI_Wtime() - time);
        save_matrix(resultant, sizes[i], sizes[i], output[i]);

        FREE(mat);
        FREE(resultant);
        FREE(mat1);
      } else {
        ring_mult_helper(sizes[i], sizes[i], sizes[i], sizes[i]);
      }
  }
  MPI_Finalize();
}

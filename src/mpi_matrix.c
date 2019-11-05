#include <stdlib.h>
#include <mpi.h>
#include "mpi_matrix.h"

float* roll_rows_base(float* row, int turn, int rows, int cols, int proc_cols) {
  int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if(world_size == 1) {
    return row;
  }
  
  int last_row = last_row_len(rows, world_size);
  int first_rows = rows / world_size;
  int last_col = last_row_len(cols, world_size);
  int first_cols = cols / world_size;

  int rec_size = (world_rank + turn + 1 != world_size - 1) ? first_rows * cols: last_row * cols;
  int send_size = (world_rank + turn + 1 == world_size - 1) ? first_rows * cols: last_row * cols;
  float* temp = malloc(rec_size * sizeof(float));
  if(world_rank == 0) {
    MPI_Recv(temp, rec_size, MPI_FLOAT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send(row, send_size, MPI_FLOAT, world_size-1, 0, MPI_COMM_WORLD);
  } else {
    MPI_Send(row, send_size, MPI_FLOAT, world_rank-1, 0, MPI_COMM_WORLD);
    MPI_Recv(temp, rec_size, MPI_FLOAT, (world_rank+1) % world_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  free(row);
  return temp;
}

int last_row_len(int n, int world_size) {
  return (n % world_size == 0) ? n / world_size : n % (world_size - 1);
}

int first_rows_len(int n, int world_size) {
  return (n % world_size == 0) ? n / world_size : n / (world_size - 1);
}

void combineResults(void* in, void* inout, int *len, MPI_Datatype *dptr) {
  int i;
  for(i=0; i < *len; ++i) {
    ((float*)inout)[i] += ((float*)in)[i];
  }
}

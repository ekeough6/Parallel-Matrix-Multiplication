#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "mpi_matrix.h"

float* roll_rows_base(float* row, int turn, int rows, int cols, MPI_Comm communicator) {
	int world_size, world_rank;
	MPI_Comm_size(communicator, &world_size);
	MPI_Comm_rank(communicator, &world_rank);
  if(world_size == 1) {
    return row;
  }
  
  int size = rows * cols;
  
  float* temp = malloc(size * sizeof(float));
  if(world_rank == 0) {
    //MPI_Recv(temp, size, MPI_FLOAT, world_size-1, 0, communicator, MPI_STATUS_IGNORE);
    //MPI_Send(row, size, MPI_FLOAT, 1, 0, communicator);
    MPI_Recv(temp, size, MPI_FLOAT, 1, 0, communicator, MPI_STATUS_IGNORE);
    MPI_Send(row, size, MPI_FLOAT, world_size-1, 0, communicator);
  } else {
    MPI_Send(row, size, MPI_FLOAT, world_rank-1, 0, communicator);
    MPI_Recv(temp, size, MPI_FLOAT, (world_rank+1) % world_size, 0, communicator, MPI_STATUS_IGNORE);
    //MPI_Send(row, size, MPI_FLOAT, (world_rank+1) % world_size, 0, communicator);
    //MPI_Recv(temp, size, MPI_FLOAT, world_rank-1, 0, communicator, MPI_STATUS_IGNORE);
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

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "ring.h"
#include "matrix.h"
#include "mpi_matrix.h"

float* ring_mult(float* mat_A, int rA, int cA, float* mat_B, int rB, int cB) {
  //initializing the MPI variables
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  float* resultant = calloc(rA * cB, sizeof(float));

  int last_row = last_row_len(rA, world_size);
  int first_rows = first_rows_len(rA, world_size);
  int last_row_size = last_row * cB * sizeof(float);
  int first_row_size = first_rows * cB * sizeof(float);

  int last_col = last_row_len(cB, world_size);
  int first_cols = first_rows_len(cB, world_size);
  int last_col_size = last_col * rA * sizeof(float);
  int first_col_size = first_cols * rA * sizeof(float);

  float* row_mat_A;
  float* col_mat_B;
  
  if(world_rank == 0) {
    //sending out the data to the other processors
    int i, j, k, row_size, col_size, rows, cols, offset;
    for(i = 1; i < world_size; ++i) {
      rows = (i != world_size - 1) ? first_rows: last_row;
      cols = (i != world_size - 1) ? first_cols: last_col;
      row_size = (i != world_size - 1) ? first_row_size : last_row_size;
      col_size = (i != world_size - 1) ? first_col_size : last_col_size;
      row_mat_A = malloc(row_size);
      col_mat_B = malloc(col_size);

      //spliting up the rows and cols between the processors
      get_row(mat_A, row_mat_A, first_rows * i, rows, cA);
      get_col(mat_B, col_mat_B, first_cols * i, rB, cols, cB); 

      MPI_Send(row_mat_A, rows * cA, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
      MPI_Send(col_mat_B, cols * rB, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
      free(row_mat_A);
      free(col_mat_B);
    }
    row_mat_A = malloc(first_row_size);
    col_mat_B = malloc(first_col_size);
    get_row(mat_A, row_mat_A, 0, first_rows, cA);
    get_col(mat_B, col_mat_B, 0, rB, first_cols, cB); 
  } else {
    //recieves the partitioned matrices from root
    int row_size = (world_rank != world_size - 1) ? first_row_size : last_row_size;
    int col_size = (world_rank != world_size - 1) ? first_col_size : last_col_size;
    int rows = (world_rank != world_size - 1) ? first_rows: last_row;
    int cols = (world_rank != world_size - 1) ? first_cols: last_col;
    row_mat_A = malloc(row_size);
    col_mat_B = malloc(col_size);
    MPI_Recv(row_mat_A, rows * cA, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(col_mat_B, cols * rB, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  //at this point both of the matrices should be properly partitioned across the processors
  int i, j, k, rows, cols;
  for(i = 0; i < world_size; ++i) {
    int r, c;
    rows = (world_rank + i != world_size - 1) ? first_rows: last_row;
    cols = (world_rank + i != world_size - 1) ? first_cols: last_col;

    //gets the partial result for the current section of the result matrix
    float* mat_C = matrix_mult(row_mat_A, rows, cA, col_mat_B, cA, cols);
    insert_matrix(resultant, mat_C, first_rows * ((world_rank + i) % (world_size)) , first_cols * world_rank, rA, cB, rows, cols);
    free(mat_C);

    row_mat_A = roll_rows(row_mat_A, i, rA, cA);
  }
  MPI_Op resultant_combine;
  MPI_Op_create(&combineResults, 1, &resultant_combine);
  float* temp = calloc(rA * cB, sizeof(float));
  MPI_Reduce(resultant, temp, rA * cB, MPI_FLOAT, resultant_combine, 0, MPI_COMM_WORLD);
  if(world_rank == 0) {
    int i;
    for(i = 0; i < rA * cB; ++i) {
      resultant[i] = temp[i];
    }
    free(temp);
    return resultant;
  }
  free(temp);
  return NULL;
}

void ring_mult_helper(int rA, int cA, int rB, int cB) {
  ring_mult(NULL, rA, cA, NULL, rB, cB);
}

float* roll_rows(float* row, int turn, int rA, int cA) {
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if(world_size == 1) {
    return row;
  }
  
  int last_row = last_row_len(rA, world_size);
  int first_rows = first_rows_len(rA, world_size);

  int rec_size = (world_rank + turn + 1 != world_size - 1) ? first_rows * cA: last_row * cA;
  int send_size = (world_rank + turn + 1 == world_size - 1) ? first_rows * cA: last_row * cA;
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

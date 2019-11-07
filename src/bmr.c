#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"
#include "mpi_matrix.h"
#include "bmr.h"

float* bmr_mult(float* mat_A, int rA, int cA, float* mat_B, int rB, int cB) {
  //initializing the MPI variables
  int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


  float* resultant = calloc(rA * cB, sizeof(float));
  float* submat_A;
  float* submat_B;
  float* submat_diag;
  
  int p_rows_A, p_cols_A, p_rows_B, p_cols_B;

  //splits mat_A into n x m submatrices
  get_dims(&p_rows_A, &p_cols_A, world_size);
  
  //splits mat_B into m x n submatrices
  get_dims(&p_cols_B, &p_rows_B, world_size);

  int first_rows = first_rows_len(rA, p_rows_A);
  int first_cols = first_rows_len(cA, p_cols_B);
  int last_row = last_row_len(rA, p_rows_A);
  int last_col = last_row_len(cA, p_cols_A);

  float* subresultant = calloc(first_rows * first_rows, sizeof(float));
  //Creating new communicators for rows of matrices (A) and cols of matrices (B)
  int row_A = world_rank / p_cols_A;
  int col_B = world_rank / p_rows_B;
  MPI_Comm row_comm, col_comm;
  MPI_Comm_split(MPI_COMM_WORLD, row_A, world_rank, &row_comm);
  MPI_Comm_split(MPI_COMM_WORLD, col_B, world_rank, &col_comm);

  int row_size, row_rank;
	MPI_Comm_size(row_comm, &row_size);
	MPI_Comm_rank(row_comm, &row_rank);

  int col_size, col_rank;
	MPI_Comm_size(col_comm, &col_size);
	MPI_Comm_rank(col_comm, &col_rank);

  if(world_rank == 0) {
    int i, j;
    //spliting up and distributing the A matrix
    for(i = 0; i < p_rows_A; ++i) {
      for(j = 0; j < p_cols_A; ++j) {
        if(i == 0 && j == 0)
          continue;
        //int rows = (i == p_rows_A - 1) ? last_row : first_rows;
        //int cols = (j == p_cols_A - 1) ? last_col : first_cols;
        int rows = first_rows;
        int cols = first_cols;
        submat_A = malloc(rows * cols * sizeof(float));
        extract_matrix_bmr(mat_A, submat_A, i, j, rA, cA, p_rows_A, p_cols_A);
        //printf("%d\n", i * p_cols_A + j);
        MPI_Send(submat_A, rows * cols, MPI_FLOAT, i * p_cols_A + j, 0, MPI_COMM_WORLD);
        free(submat_A);
      }
    }
    for(i = 0; i < p_rows_B; ++i) {
      for(j = 0; j < p_cols_B; ++j) {
        if(i == 0 && j == 0)
          continue;
        //int cols = (i == p_rows_B - 1) ? last_row : first_rows;
        //int rows = (j == p_cols_B - 1) ? last_col : first_cols;
        int rows = first_rows;
        int cols = first_cols;
        submat_B = malloc(rows * cols * sizeof(float));
        extract_matrix_bmr(mat_B, submat_B, i, j, rB, cB, p_cols_B, p_rows_B);
        //printf("%d\n", i * p_cols_B + j);
        MPI_Send(submat_A, rows * cols, MPI_FLOAT, i * p_cols_B + j, 0, MPI_COMM_WORLD);
        free(submat_B);
      }
    }
    submat_A = malloc(first_rows * first_cols * sizeof(float));
    submat_B = malloc(first_rows * first_cols * sizeof(float));
    extract_matrix_bmr(mat_A, submat_A, row_A, row_rank, rA, cA, p_rows_A, p_cols_A);
    extract_matrix_bmr(mat_B, submat_B, col_rank, col_B, rA, cA, p_rows_A, p_cols_A);
  } else {
    //retrieve the matrices from the root node
    //int rows = (row_A == p_rows_A - 1) ? last_row : first_rows;
    //int cols = (row_rank == p_cols_A - 1) ? last_col : first_cols;
    int rows = first_rows;
    int cols = first_cols;
    submat_A = malloc(rows * cols * sizeof(float));
    submat_B = malloc(rows * cols * sizeof(float));
    MPI_Recv(submat_A, rows * cols, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //printf("idk\n");
    MPI_Recv(submat_B, rows * cols, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  int i, j;
  for(i = 0; i < p_cols_A; ++i) {
    //bcast diagonal element
    int size = first_rows * first_cols;
    submat_diag = malloc(size * sizeof(float));
    if((row_rank + i) % row_size == row_A) {
      for(j = 0; j < size; ++j) {
        submat_diag[j] = submat_A[j];
      }
    }
    MPI_Bcast(submat_diag, size, MPI_FLOAT, (i + row_A) % row_size, row_comm);
    if(row_rank < row_size - (p_cols_A - p_rows_A)) {
      float* temp_res = matrix_mult(submat_A, first_rows, first_cols, submat_B, first_cols, first_rows);
      add_matrices(subresultant, temp_res, first_rows, first_rows);
      free(temp_res);
    }
    submat_B = roll_rows_base(submat_B, i, first_cols, first_rows, col_comm);
  }
  
  if(row_rank < row_size - (p_cols_A - p_rows_A)) {
    insert_matrix(resultant, subresultant, row_A * first_rows, row_rank * first_rows, rA, cB, first_rows, first_rows);
  }
  float* temp = calloc(rA * cB, sizeof(float));
  free(submat_B);
  free(submat_A);
  free(subresultant);
  free(submat_diag);
  MPI_Comm_free(&row_comm);
  MPI_Comm_free(&col_comm);
  MPI_Op resultant_combine;
  MPI_Op_create(&combineResults, 1, &resultant_combine);
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

void bmr_mult_helper(int rA, int cA, int rB, int cB) {
  bmr_mult(NULL, rA, cA, NULL, rB, cB);
}

void get_dims(int* p_rows, int* p_cols, int world_size) {
  int r = sqrt(world_size);
  while(world_size % r > 0) {
    r--;
  }
  *p_cols = world_size / r;
  *p_rows = r;
}

void extract_matrix_bmr(float* matrix, float* output, int p_row, int p_col, int rows, int cols, int world_rows, int world_cols) {
  int first_rows = first_rows_len(rows, world_rows);
  int first_cols = first_rows_len(cols, world_cols);
  int last_row = last_row_len(rows, world_rows);
  int last_col = last_row_len(cols, world_cols);
  int row_offset = p_row * first_rows;
  int col_offset = p_col * first_cols;
  int extract_rows = (p_row == world_rows - 1) ? last_row : first_rows;
  int extract_cols = (p_col == world_cols - 1) ? last_row : first_rows;
  extract_matrix(matrix, output, row_offset, col_offset, extract_rows, extract_cols, cols);
}

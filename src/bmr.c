#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"
#include "mpi_matrix.h"
#include "bmr.h"

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
  int extract_rows = (p_rows == world_rows - 1) ? last_row : first_rows;
  int extract_cols = (p_cols == world_cols - 1) ? last_row : first_rows;

  extract_matrix(matrix, output, row_offset, col_offset, extract_rows, extract_cols, cols);
}

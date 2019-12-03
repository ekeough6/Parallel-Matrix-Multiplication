#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "matrix.h"

void mmalloc(void** p, size_t size) {
  #ifdef USE_GPU
  cudaMallocHost(p, size);  
  #else
  (*p) = malloc(size);
  #endif
}

float* matrix_mult(float* mat_A, int rA, int cA, float* mat_B, int rB, int cB) {
  if (cA != rB) {
    return NULL;
  }
  int i, j, k;
  float *resultant;// = malloc(rA * cB * sizeof(float));
  mmalloc((void **)&resultant, rA * cB * sizeof(float));
  for(i = 0; i < rA * cB; ++i) {
    resultant[i] = 0;
  }
#ifdef USE_GPU
  gpu_matrix_mult(mat_A, mat_B, resultant, rA, cA, cB);
  return resultant;
#else
  for(i = 0; i < rA; ++i) {
    for(j = 0; j < cB; ++j) {
      for(k = 0; k < rB; ++k) {
        resultant[i * cB + j] += mat_A[i * cA + k] * mat_B[k * cB + j];
      }
    }
  }
  return resultant;
#endif
}

void add_matrices(float* mat_A, float* mat_B, int row, int col) {
  int i,j;
  for(i = 0; i < row; ++i) {
    for(j = 0; j < col; ++j) {
      mat_A[i * col + j] += mat_B[i * col + j];
    }
  }
}

float dot_prod(float* vec_A, float* vec_B, int n) {
  float sum = 0;
  int i;
  for(i = 0; i < n; ++i) {
    sum += vec_A[i] * vec_B[i];
  }
  return sum;
}

float dot_prod_col(float* row_A, float* mat_B, int rows, int col, int n) {
  float sum = 0;
  int i;
  for(i = 0; i < n; ++i) {
    sum += row_A[i] * mat_B[col * rows + i];
  }
  return sum;
}

float* generate_matrix(int rows, int cols) {
  int i, j;
  float* matrix; //= malloc(rows * cols * sizeof(float));
  mmalloc((void **)&matrix, rows * cols * sizeof(float));
  for(i = 0; i < rows; ++i) {
    for(j = 0; j < cols; ++j) {
      matrix[i * cols + j] = (float)rand() / RAND_MAX * 2 - 1;
    }
  }
  return matrix; 
}

void save_matrix(float* mat, int rows, int cols, char* filename) {
  int i, j;
  FILE* file = fopen(filename, "w");
  fprintf(file, "%d %d\n", rows, cols);
  for(i = 0; i < rows; ++i) {
    for(j = 0; j < cols; ++j) {
      fprintf(file, "%6.3f ", mat[i * cols + j]);
    }
    fprintf(file, "\n");
  }
  fclose(file);
} 

float* load_matrix(char* filename, int* rows, int* cols) {
  int i, j;
  FILE* file = fopen(filename, "r");
  fscanf(file, "%d %d\n", rows, cols);
  float* mat;// = malloc(*rows * *cols * sizeof(float));
  mmalloc((void**)&mat, *rows * *cols * sizeof(float));
  for(i = 0; i < *rows; ++i) {
    for(j = 0; j < *cols; ++j) {
      fscanf(file, "%f ", mat + i * *cols + j);
    }
  }
  fclose(file);
  return mat;
}

void get_row(float* mat, float* mat_row, int row_offset, int rows, int cols) {
  //float* mat_row = malloc(cols * rows * sizeof(float));
  int offset = cols * row_offset;
  int i, j;
  for(i = 0; i < rows; ++i) {
    for(j = 0; j < cols; ++j) {
      mat_row[i * cols + j] = mat[offset + i * cols + j];
    }
  }
}

void get_col(float* mat, float* mat_col, int col_offset, int rows, int cols, int total_cols) {
  int i, j;
  for(i = 0; i < rows; ++i) {
    for(j = 0; j < cols; ++j) {
      mat_col[i * cols + j] = mat[i * total_cols + j + col_offset];
    }
  }
}

void insert_matrix(float* mat_A, float* mat_B, int row_offset, int col_offset, int rA, int cA, int rB, int cB) {
  int i, j;
  for(i = 0; i < rB; ++i) {
    for(j = 0; j < cB; ++j) {
      mat_A[(row_offset + i) * cA + col_offset + j] = mat_B[i * cB + j];
    }
  }
}

void extract_matrix(float* matrix, float* output, int row_offset, int col_offset, int rows, int cols, int total_cols) {
  int i, j;
  for(i = 0; i < rows; ++i) {
    for(j = 0; j < cols; ++j) {
      output[i * cols + j] = matrix[(row_offset + i) * total_cols + col_offset + j];
    }
  }
}

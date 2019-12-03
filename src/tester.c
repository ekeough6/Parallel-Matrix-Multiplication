#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int main(int argc,  char** argv) {
  int i, j;
  float *mat_A; //= malloc(5 * sizeof(float) * 5);
  mmalloc((void**)&mat_A, 5 * sizeof(float) * 5);
  for(i = 0; i < 5; i++) {
    for(j = 0; j < 5; j++) {
      mat_A[i * 5 + j] = 1;//(i == j) ? 1 : 0;
    }
  }

  float *mat_B;// = malloc(5 * sizeof(float) * 5);
  mmalloc((void**)mat_B, 5 * sizeof(float) * 5);
  for(i = 0; i < 5; i++) {
    for(j = 0; j < 5; j++) {
      mat_B[i * 5 + j] = 1;
    }
  }

  //printf("Identity mult\n");
  float* resultant = matrix_mult(mat_A, 5, 5, mat_B, 5, 5);
  if (NULL != resultant) {
    for(i = 0; i < 5; i++) {
      for(j = 0; j < 5; j++) {
        printf("%2.2f\t", resultant[i * 5 + j]);
      }
      printf("\n");
    }
  }
  printf("\n");

  /*float* mat = generate_matrix(5, 5);
  for(i = 0; i < 5; i++) {
    for(j = 0; j < 5; j++) {
      printf("%2.2f\t", mat[i * 5 + j]);
    }
    printf("\n");
  }
  printf("\n");

  save_matrix(mat_A, 5, 5, "identity");

  int rows, cols;
  float* matrix = load_matrix("identity", &rows, &cols);
  for(i = 0; i < 5; i++) {
    for(j = 0; j < 5; j++) {
      printf("%2.2f\t", matrix[i * 5 + j]);
    }
    printf("\n");
  }
  printf("\n");

  float* mat_m = generate_matrix(2, 2);
  for(i = 0; i < 2; i++) {
    for(j = 0; j < 2; j++) {
      printf("%2.2f\t", mat_m[i * 2 + j]);
    }
    printf("\n");
  }
  printf("\n");

  insert_matrix(matrix, mat_m, 2, 2, 5, 5, 2, 2);

  for(i = 0; i < 5; i++) {
    for(j = 0; j < 5; j++) {
      printf("%2.2f\t", matrix[i * 5 + j]);
    }
    printf("\n");
  }
  printf("\n");

  for(i = 0; i < 2; i++) {
    for(j = 0; j < 2; j++) {
      mat_m[i * 2 + j] = 0;
      printf("%2.2f\t", mat_m[i * 2 + j]);
    }
    printf("\n");
  }
  printf("\n");

  extract_matrix(matrix, mat_m, 2, 2, 2, 2, 5);
  for(i = 0; i < 2; i++) {
    for(j = 0; j < 2; j++) {
      printf("%2.2f\t", mat_m[i * 2 + j]);
    }
    printf("\n");
  }
  printf("\n");*/


  FREE(mat_A);
  FREE(mat_B);
  FREE(resultant);
  //free(mat);
  //free(matrix);
  //free(mat_m);
  return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int main(int argc, char** argv) {
  time_t t;
  srand((unsigned) time(&t));
  int sizes[3] = {(1<<4) * 24, (1<<6) * 24, (1<<8) * 24};
  char* input[3] = {"input0.txt", "input1.txt", "input2.txt"};
  char* input2[3] = {"input00.txt", "input11.txt", "input22.txt"};
  char* output[3] = {"result0.txt", "result1.txt", "result2.txt"};
  int i, j, k;
  for(i = 0; i < 3; ++i) {
    printf("Generating matrices: %d\n", i);
    float* mat = generate_matrix(sizes[i], sizes[i]);
    save_matrix(mat, sizes[i], sizes[i], input[i]);
    float* mat1 = generate_matrix(sizes[i], sizes[i]);
    save_matrix(mat1, sizes[i], sizes[i], input2[i]);
    float* result = matrix_mult(mat, sizes[i], sizes[i], mat1, sizes[i], sizes[i]);
    save_matrix(result, sizes[i], sizes[i], output[i]);
    FREE(mat);
  }
  return 0;
}

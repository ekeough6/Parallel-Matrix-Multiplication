#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matrix.h"

int main(int argc, char** argv) {
  time_t t;
  srand((unsigned) time(&t));
  int sizes[3] = {(1<<8), (1<<10), (1<<12)};
  char* input[3] = {"sinput0.txt", "sinput1.txt", "sinput2.txt"};
  char* input2[3] = {"sinput00.txt", "sinput11.txt", "sinput22.txt"};
  char* output[3] = {"sresult0.txt", "sresult1.txt", "sresult2.txt"};
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
    FREE(mat1);
  }
  return 0;
}

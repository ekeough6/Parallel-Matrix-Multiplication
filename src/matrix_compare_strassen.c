#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int main(int argc, char** argv) {
  int i, r, c;
  int sizes[3] = {(1<<8), (1<<10), (1<<12)};
  char* input[3] = {"sresult0.txt", "sresult1.txt", "sresult2.txt"};
  char* strass[3] = {"strassen_output0.txt", "strassen_output1.txt", "strassen_output2.txt"};
  float *in_mat, *strass_mat;
  for(i = 0; i < 3; ++i) {
    int in_row, in_col, strass_row, strass_col;
    in_mat = load_matrix(input[i], &in_row, &in_col);
    strass_mat = load_matrix(strass[i], &strass_row, &strass_col);

    int diffs = 0;
    int diff;

    diffs = 0;
    if(in_row == strass_row && in_col == strass_col) {
      for(r = 0; r < in_row; ++r) {
        for(c = 0; c < in_col; ++c) {
          diff = in_mat[r * in_col + c] - strass_mat[r * in_col + c]; 
          diff = (diff < 0) ? -diff : diff;
          diffs = (diff < 0.1) ? diff : diff + 1;
        }
      }
    }
    printf("Strassen %d: %d differences\n", i, diffs);

    FREE(in_mat);
    FREE(strass_mat);
  }
  return 0;
}

    

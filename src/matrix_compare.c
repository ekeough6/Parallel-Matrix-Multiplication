#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int main(int argc, char** argv) {
  int i, r, c;
  int sizes[3] = {(1<<4) * 24, (1<<6) * 24, (1<<8) * 24};
  char* input[3] = {"result0.txt", "result1.txt", "result2.txt"};
  char* bmr[3] = {"bmr_output0.txt", "bmr_output1.txt", "bmr_output2.txt"};
  char* ring[3] = {"ring_output0.txt", "ring_output1.txt", "ring_output2.txt"};
  float *in_mat, *ring_mat, *bmr_mat;
  for(i = 0; i < 3; ++i) {
    int in_row, in_col, bmr_row, bmr_col, ring_row, ring_col;
    in_mat = load_matrix(input[i], &in_row, &in_col);
    ring_mat = load_matrix(ring[i], &ring_row, &ring_col);
    bmr_mat = load_matrix(bmr[i], &bmr_row, &bmr_col);

    int diffs = 0;
    int diff;
    if(in_row == ring_row && in_col == ring_col) {
      for(r = 0; r < in_row; ++r) {
        for(c = 0; c < in_col; ++c) {
          diff = in_mat[r * in_col + c] - ring_mat[r * in_col + c]; 
          diff = (diff < 0) ? -diff : diff;
          diffs = (diff < 0.001) ? diff : diff + 1;
        }
      }
    }
    printf("Ring %d: %d differences\n", i, diffs);

    diffs = 0;
    if(in_row == bmr_row && in_col == bmr_col) {
      for(r = 0; r < in_row; ++r) {
        for(c = 0; c < in_col; ++c) {
          diff = in_mat[r * in_col + c] - bmr_mat[r * in_col + c]; 
          diff = (diff < 0) ? -diff : diff;
          diffs = (diff < 0.001) ? diff : diff + 1;
        }
      }
    }
    printf("BMR %d: %d differences\n", i, diffs);

    FREE(in_mat);
    FREE(ring_mat);
    FREE(bmr_mat);
  }
  return 0;
}

    

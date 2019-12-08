#ifndef STRASSEN_H
#define STRASSEN_H

float* strassen_mult(float* mat_A, float* mat_B, int rows, int cols);

void strassen_mult_helper(int rows, int cols);

void strassen_extract(float *mat_A, float *mat_B, int rows, int cols, float** sub_As, float** sub_Bs);

#endif

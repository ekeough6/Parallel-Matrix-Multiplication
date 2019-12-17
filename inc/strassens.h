#ifndef STRASSEN_H
#define STRASSEN_H

float* strassen_mult(float* mat_A, float* mat_B, int size, int root, int iter);

void strassen_mult_helper(int size, int root, int iter);

void strassen_extract(float *mat_A, float *mat_B, int rows, int cols, float** sub_As, float** sub_Bs);

void strassen_combine(float *resultant, float **m_mats, int size);

float* l_strassen_mult(float* mat_A, float *mat_B, int size, int iter);
#endif

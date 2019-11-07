#ifndef BMR_H
#define BMR_H

//multiples the given matrices and puts the result in resultant
float* bmr_mult(float* mat_A, int rA, int cA, float* mat_B, int rB, int cB);

void get_dims(int* p_rows, int* p_cols, int world_size);

void bmr_mult_helper(int rA, int cA, int rB, int cB);

void extract_matrix_bmr(float* matrix, float* output, int p_row, int p_col, int rows, int cols, int world_rows, int world_cols);
#endif

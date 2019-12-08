#ifndef MATRIX_H
#define MATRIX_H

#ifdef USE_GPU
#define FREE cudaFree
#else
#define FREE free
#endif

void mmalloc(void** p, size_t size);

//Multiplies two matrices given that cA == rB
float* matrix_mult(float* mat_A, int rA, int cA, float* mat_B, int rB, int cB);

void add_matrices(float* mat_A, float* mat_B, int row, int col);

void matrix_sum(float* resultant, float* mat_A, float* mat_B, int row, int col);

void matrix_diff(float* resultant, float* mat_A, float* mat_B, int row, int col);

//calculates the dot product of two matrices of size n
float dot_prod(float* vec_A, float* vec_B, int n);

//calculates dot product of row A and col of mat_B, both of size n
float dot_prod_col(float* row_A, float* mat_B, int rows, int col, int n);

//randomly generate a matrix of the given dimensions
float* generate_matrix(int rows, int cols);

//save a matrix to a file
void save_matrix(float* mat, int rows, int cols, char* filename);

//load a matrix from a file
float* load_matrix(char* filename, int* rows, int* cols);

void get_row(float* mat, float* mat_row, int row_offset, int rows, int cols);

void get_col(float* mat, float* mat_col, int col_offset, int rows, int cols, int total_cols);

void insert_matrix(float* mat_A, float* mat_B, int row_offset, int col_offset, int rA, int cA, int rB, int cB); 

void extract_matrix(float* matrix, float* output, int row_offset, int col_offset, int rows, int cols, int total_cols);

void gpu_matrix_mult(float* a, float* b, float* c, int m, int n, int k);

/*__global__
void matrix_mult_GPU(float** mat_A, float** mat_B, float** mat_C, int a, int b, int c);

__global__
float dot_prod_GPU(float* vec_A, float* vec_B, int n);

__global__
float dot_prod_col_GPU(float* row_A, float** mat_B, int col, int n);
*/
#endif

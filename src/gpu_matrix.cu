#include <cuda.h>
#include <cuda_runtime.h>
__global__ void gpu_matrix_multiply(float* a,float* b,float* c, int m, int n, int k)
  { 
  int row = blockIdx.y * blockDim.y + threadIdx.y; 
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  int sum = 0;
  int i;
  if( col < k && row < m) 
  {
    for(i = 0; i < n; i++) 
    {
      sum += a[row * n + i] * b[i * k + col];
    }
    c[row * k + col] = sum;
  }
}

extern "C" void gpu_matrix_mult(float* a, float* b, float* c, int m, int n, int k) {
  float *d_a, *d_b, *d_c;
  cudaMalloc((void **) &d_a, sizeof(float)*m*n);
  cudaMalloc((void **) &d_b, sizeof(float)*n*k);
  cudaMalloc((void **) &d_c, sizeof(float)*m*k);
  cudaMemcpy(d_a, a, sizeof(float)*m*n, cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, b, sizeof(float)*n*k, cudaMemcpyHostToDevice);
  unsigned int grid_rows = (m + BLOCK_SIZE - 1) / BLOCK_SIZE;
  unsigned int grid_cols = (k + BLOCK_SIZE - 1) / BLOCK_SIZE;
  dim3 dimGrid(grid_cols, grid_rows);
  dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);
  gpu_matrix_multiply<<<dimGrid, dimBlock>>>(d_a, d_b, d_c, m, n, k);
  cudaMemcpy(c, d_c, sizeof(float)*m*k, cudaMemcpyDeviceToHost);
}

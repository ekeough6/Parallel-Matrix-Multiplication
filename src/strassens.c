#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "matrix.h"
#include "mpi_matrix.h"
#include "strassens.h"
#define MMS 7
#define BASE_TAG 0x00FFFFFF
#define ELEMENT_TAG 0xFF000000

void sort(int a[], int array_size) {
  int i, j, temp;
  for (i = 0; i < (array_size - 1); ++i) {
    for (j = 0; j < array_size - 1 - i; ++j ) {
      if (a[j] > a[j+1]) {
        temp = a[j+1];
        a[j+1] = a[j];
        a[j] = temp;
      }
    }
  }
}

int contains(int a[], int size, int val) {
  int i;
  for(i = 0; i < size; ++i) {
    if(a[i] == val)
      return 1;
  }
  return 0;
}

float* strassen_mult(float* mat_A, float* mat_B, int size, int root, int iter) {
  int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if(world_size == 1) {
    return l_strassen_mult(mat_A, mat_B, size, 0);
  }

  const int size_1 = size / 2; //at this level tag is always 0
  const int size_2 = size_1 / 2; //at this level tag is world_rank_1+1
  const int size_3 = size_2 / 2; //at this level tag is world_rank_1+1<<7 + world_rank_2+1
  const int size_4 = size_3 / 2; //at this level tag is world_rank_1+1<<14 + world_rank_2+1<<7 + world_rank_3+1
  const int sizes[] = {size, size_1, size_2, size_3, size_4};
  //const int base_rank = world_rank * 7 % world_size;

  float *resultant, ***sub_As, ***sub_Bs, *recv_A, *recv_B, ***m_mats, *send_m;
  int destinations[MMS]; 
  int sources[MMS]; 
  int other_sources[MMS]; 
  int i, j;
  int recv_size;
  resultant = NULL;
  const int divisions[] = {7, 7*7, 7*7*7, 7*7*7*7};

  
  
  //divide into all of the sub matrices until tree level >= world_size
  //send out sub matrices
  //further divide locally
  //multiply locally
  //combine to original level
  //send up
  //send out remainder
  //further divide locally
  //multiply locally
  //combine to original level
  //send up
  //combine to base level
  
  int spl_level;
  for(i = 0; i < 4; ++i) {
    if(divisions[i] >= world_size) {
      spl_level = i;
      break;
    }
  }

  if(world_rank == 0) {
    mmalloc((void**)&sub_As, 4 * sizeof(float**));
    mmalloc((void**)&sub_Bs, 4 * sizeof(float**));
    mmalloc((void**)&m_mats, 4 * sizeof(float**));
    for(i = 0; i < 4; ++i) {
      mmalloc((void**)&sub_As[i], divisions[i] * sizeof(float*));
      mmalloc((void**)&sub_Bs[i], divisions[i] * sizeof(float*));
      mmalloc((void**)&m_mats[i], divisions[i] * sizeof(float*));
    }
    for(i = 0; i <= spl_level; ++i) {
      for(j = 0; j < divisions[i]; ++j) {
        mmalloc((void**)&sub_As[i][j], sizes[i+1] * sizes[i+1] * sizeof(float));
        mmalloc((void**)&sub_Bs[i][j], sizes[i+1] * sizes[i+1] * sizeof(float));
        mmalloc((void**)&m_mats[i][j], sizes[i+1] * sizes[i+1] * sizeof(float));
      }
      if(i == 0) {
        strassen_extract(mat_A, mat_B, size, size, sub_As[i], sub_Bs[i]);
      } else {
        for(j = 0; j < divisions[i-1]; ++j) {
          strassen_extract(sub_As[i-1][j], sub_Bs[i-1][j], sizes[i], sizes[i], sub_As[i] + 7 * j, sub_Bs[i] + 7 * j); //not sure if this will extract properly
        }
      }
    }
  } else {
    mmalloc((void**)&recv_A, sizes[spl_level+1] * sizes[spl_level+1] * sizeof(float));
    mmalloc((void**)&recv_B, sizes[spl_level+1] * sizes[spl_level+1] * sizeof(float));
  }

  int offset = 0;
  while(offset < divisions[spl_level]) {
    if(world_rank == 0) {
      for(i = 1; i < world_size && i + offset < divisions[spl_level]; ++i) {
        MPI_Send(sub_As[spl_level][offset + i], sizes[spl_level+1]*sizes[spl_level+1], MPI_FLOAT, i, 0, MPI_COMM_WORLD);
        MPI_Send(sub_Bs[spl_level][offset + i], sizes[spl_level+1]*sizes[spl_level+1], MPI_FLOAT, i, 0, MPI_COMM_WORLD);
      }
      //time to locally multiply using strassens
      m_mats[spl_level][offset] = l_strassen_mult(sub_As[spl_level][offset], sub_Bs[spl_level][offset], sizes[spl_level+1], spl_level);
      for(i = 1; i < world_size && i + offset < divisions[spl_level]; ++i) {
        MPI_Recv(m_mats[spl_level][offset + i], sizes[spl_level+1]*sizes[spl_level+1], MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    } else if(world_rank + offset < divisions[spl_level]) {
      MPI_Recv(recv_A, sizes[spl_level+1]*sizes[spl_level+1], MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(recv_B, sizes[spl_level+1]*sizes[spl_level+1], MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      send_m = l_strassen_mult(recv_A, recv_B, sizes[spl_level+1], spl_level);
      MPI_Send(send_m, sizes[spl_level+1]*sizes[spl_level+1], MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
      FREE(send_m);
    }
    offset += world_size;
  }

  if(world_rank == 0) {
    //time to combine the remainder of the m_mats
    mmalloc((void**)&resultant, size * size * sizeof(float));
    for(i = spl_level; i >= 0; --i) {
      if(i > 0) {
        for(j = 0; j < divisions[i-1]; ++j) {
            strassen_combine(m_mats[i-1][j], m_mats[i] + 7 * j, sizes[i]); //gotta combine somehow
        }
      } else {
        strassen_combine(resultant, m_mats[0], size);
      }
    }
    for(i = 0; i <= spl_level; ++i) {
      for(j = 0; j < divisions[i]; ++j) {
        FREE(sub_As[i][j]);
        FREE(sub_Bs[i][j]);
        FREE(m_mats[i][j]);
      }
      FREE(sub_As[i]);
      FREE(sub_Bs[i]);
    }
    FREE(sub_As);
    FREE(sub_Bs);
  } else {
    FREE(recv_A);
    FREE(recv_B);
  }

  return resultant;
}

void strassen_mult_helper(int size, int root, int iter) {
  if(iter >= 4)
    return;
  strassen_mult(NULL, NULL, size, root, iter);
}

void strassen_extract(float *mat_A, float *mat_B, int rows, int cols, float** sub_As, float** sub_Bs) {
  float **a_quad, **b_quad;
  mmalloc((void**)&a_quad, 4 * sizeof(float*));
  mmalloc((void**)&b_quad, 4 * sizeof(float*));
  int i, j;
  for(i = 0; i < 4; ++i) {
    mmalloc((void**)&a_quad[i], rows/2 * cols/2 * sizeof(float));
    mmalloc((void**)&b_quad[i], rows/2 * cols/2 * sizeof(float));
  }
  for(i = 0; i < 2; ++i) {
    for(j = 0; j < 2; ++j) {
      extract_matrix(mat_A, a_quad[i * 2 + j], rows/2 * i, cols/2 * j, rows/2, cols/2, cols);
      extract_matrix(mat_B, b_quad[i * 2 + j], rows/2 * i, cols/2 * j, rows/2, cols/2, cols);
    }
  }
  matrix_sum(sub_As[0], a_quad[0], a_quad[3], rows/2, cols/2);
  matrix_sum(sub_As[1], a_quad[2], a_quad[3], rows/2, cols/2);
  memcpy(sub_As[2], a_quad[0], rows * cols / 4 * sizeof(float));
  memcpy(sub_As[3], a_quad[3], rows * cols / 4 * sizeof(float));
  matrix_sum(sub_As[4], a_quad[0], a_quad[1], rows/2, cols/2);
  matrix_diff(sub_As[5], a_quad[2], a_quad[0], rows/2, cols/2);
  matrix_diff(sub_As[6], a_quad[1], a_quad[3], rows/2, cols/2);
   
  matrix_sum(sub_Bs[0], b_quad[0], b_quad[3], rows/2, cols/2);
  memcpy(sub_Bs[1], b_quad[0], rows * cols / 4 * sizeof(float));
  matrix_diff(sub_Bs[2], b_quad[1], b_quad[3], rows/2, cols/2);
  matrix_diff(sub_Bs[3], b_quad[2], b_quad[0], rows/2, cols/2);
  memcpy(sub_Bs[4], b_quad[3], rows * cols / 4 * sizeof(float));
  matrix_sum(sub_Bs[5], b_quad[0], b_quad[1], rows/2, cols/2);
  matrix_sum(sub_Bs[6], b_quad[2], b_quad[3], rows/2, cols/2);
  for(i = 0; i < 4; ++i) {
    FREE(a_quad[i]);
    FREE(b_quad[i]);
  }
  FREE(a_quad);
  FREE(b_quad);
}

void strassen_combine(float *resultant, float **m_mats, int size) {
  float *c11, *c12, *c21, *c22;
  mmalloc((void**)&c11, size/2 * size/2 * sizeof(float));
  mmalloc((void**)&c12, size/2 * size/2 * sizeof(float));
  mmalloc((void**)&c21, size/2 * size/2 * sizeof(float));
  mmalloc((void**)&c22, size/2 * size/2 * sizeof(float));

  matrix_sum(c11, m_mats[0], m_mats[3], size/2, size/2);
  matrix_diff(c11, c11, m_mats[4], size/2, size/2);
  matrix_sum(c11, c11, m_mats[6], size/2, size/2);

  matrix_sum(c12, m_mats[2], m_mats[4], size/2, size/2);

  matrix_sum(c21, m_mats[1], m_mats[3], size/2, size/2);

  matrix_sum(c22, m_mats[0], m_mats[2], size/2, size/2);
  matrix_diff(c22, c22, m_mats[1], size/2, size/2);
  matrix_sum(c22, c22, m_mats[5], size/2, size/2);
  
  insert_matrix(resultant, c11, 0, 0, size, size, size/2, size/2);
  insert_matrix(resultant, c12, 0, size/2, size, size, size/2, size/2);
  insert_matrix(resultant, c21, size/2, 0, size, size, size/2, size/2);
  insert_matrix(resultant, c22, size/2, size/2, size, size, size/2, size/2);

  FREE(c11);
  FREE(c12);
  FREE(c21);
  FREE(c22);
}

float* l_strassen_mult(float* mat_A, float *mat_B, int size, int iter) {
  if(iter == 4) {
    return matrix_mult(mat_A, size, size, mat_B, size, size);
  }
  float **sub_As, **sub_Bs, **m_mats, *resultant;
  int i;
  mmalloc((void**)&sub_As, 7 * sizeof(float*));
  mmalloc((void**)&sub_Bs, 7 * sizeof(float*));
  mmalloc((void**)&m_mats, 7 * sizeof(float*));
  for(i = 0; i < MMS; ++i) {
    mmalloc((void**)&sub_As[i], size * size / 4 * sizeof(float));
    mmalloc((void**)&sub_Bs[i], size * size / 4 * sizeof(float));
  }
  strassen_extract(mat_A, mat_B, size, size, sub_As, sub_Bs);
  for(i = 0; i < MMS; ++i) {
    m_mats[i] = l_strassen_mult(sub_As[i], sub_Bs[i], size/2, iter+1);
  }
  mmalloc((void**)&resultant, size * size * sizeof(float));
  strassen_combine(resultant, m_mats, size);
  return resultant;
}

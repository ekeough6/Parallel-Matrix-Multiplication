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
  printf("%d, %d iter %d\n", world_rank, root, iter);

  const int size_1 = size / 2; //at this level tag is always 0
  const int size_2 = size_1 / 2; //at this level tag is world_rank_1+1
  const int size_3 = size_2 / 2; //at this level tag is world_rank_1+1<<7 + world_rank_2+1
  const int size_4 = size_3 / 2; //at this level tag is world_rank_1+1<<14 + world_rank_2+1<<7 + world_rank_3+1
  const int sizes[] = {size, size_1, size_2, size_3, size_4};
  //const int base_rank = world_rank * 7 % world_size;
  if(iter == 4) {
    return matrix_mult(mat_A, size_4, size_4, mat_B, size_4, size_4);
  }

  float *resultant, **sub_As, **sub_Bs, *recv_A, *recv_B, **m_mats;
  int destinations[MMS]; 
  int sources[MMS]; 
  int other_sources[MMS]; 
  int i, j;
  int recv_size;
  resultant = NULL;

  mmalloc((void**)&sub_As, MMS * sizeof(float*));
  mmalloc((void**)&sub_Bs, MMS * sizeof(float*));
  mmalloc((void**)&m_mats, MMS * sizeof(float*));

  for(i = 0; i < MMS; ++i) {
    destinations[i] = (world_rank + 8 * i) % world_size;
    sources[i] = (root + 8 * i) % world_size;
    other_sources[i] = (world_rank - 8 * i) % world_size;
    other_sources[i] = (other_sources[i] < 0) ? world_size + other_sources[i] : other_sources[i];
    if(world_rank == 1)
      printf("dest: %d, source: %d\n", destinations[i], sources[i]);
    mmalloc((void**)&sub_As[i], sizes[iter+1] * sizes[iter+1] * sizeof(float));
    mmalloc((void**)&sub_Bs[i], sizes[iter+1] * sizes[iter+1] * sizeof(float));
    mmalloc((void**)&m_mats[i], sizes[iter+1] * sizes[iter+1] * sizeof(float));
  }
  sort(destinations, MMS);
  sort(sources, MMS);
  sort(other_sources, MMS);

  if(world_rank == root) {
    strassen_extract(mat_A, mat_B, sizes[iter], sizes[iter], sub_As, sub_Bs);
  }

  mmalloc((void**)&recv_A, sizes[iter+1] * sizes[iter+1] * sizeof(float));
  mmalloc((void**)&recv_B, sizes[iter+1] * sizes[iter+1] * sizeof(float));

  if(world_rank == root) {
    int index = 0;
    for(i = 0; i < MMS; ++i) {
      if(destinations[i] == world_rank) {
        index = i;
        continue;
      }
      printf("sending out some shit\n");
      MPI_Send(sub_As[i], sizes[iter+1] * sizes[iter+1], MPI_FLOAT, i, 0, MPI_COMM_WORLD);
      MPI_Send(sub_Bs[i], sizes[iter+1] * sizes[iter+1], MPI_FLOAT, i, 0, MPI_COMM_WORLD);
    }
    m_mats[index] = strassen_mult(sub_As[index], sub_Bs[index], size, root, iter+1);
    for(i = 0; i < MMS; ++i) {
      printf("os %d\n", other_sources[i]);
      if(other_sources[i] == world_rank) continue;
      strassen_mult_helper(size, other_sources[i], iter+1);
    }
    printf("pizza time %d\n", world_rank);
    for(i = 0; i < MMS; ++i) {
      if(i == index) continue; //skip over recv from same node
      MPI_Recv(m_mats[i], sizes[iter+1] * sizes[iter+1], MPI_FLOAT, destinations[i], 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    mmalloc((void**)&resultant, sizes[iter] * sizes[iter] * sizeof(float));
    float *c11, *c12, *c21, *c22;

    mmalloc((void**)&c11, sizes[iter+1] * sizes[iter+1] * sizeof(float));
    mmalloc((void**)&c12, sizes[iter+1] * sizes[iter+1] * sizeof(float));
    mmalloc((void**)&c21, sizes[iter+1] * sizes[iter+1] * sizeof(float));
    mmalloc((void**)&c22, sizes[iter+1] * sizes[iter+1] * sizeof(float));

    matrix_sum(c11, m_mats[0], m_mats[3], sizes[iter+1], sizes[iter+1]);
    matrix_diff(c11, c11, m_mats[4], sizes[iter+1], sizes[iter+1]);
    matrix_sum(c11, c11, m_mats[6], sizes[iter+1], sizes[iter+1]);

    matrix_sum(c12, m_mats[2], m_mats[4], sizes[iter+1], sizes[iter+1]);

    matrix_sum(c21, m_mats[1], m_mats[3], sizes[iter+1], sizes[iter+1]);

    matrix_sum(c11, m_mats[0], m_mats[2], sizes[iter+1], sizes[iter+1]);
    matrix_diff(c11, c11, m_mats[1], sizes[iter+1], sizes[iter+1]);
    matrix_sum(c11, c11, m_mats[5], sizes[iter+1], sizes[iter+1]);
    
    printf("Time to combine some magic\n");
    insert_matrix(resultant, c11, 0, 0, sizes[iter], sizes[iter], sizes[iter+1], sizes[iter+1]);
    insert_matrix(resultant, c12, 0, sizes[iter+1], sizes[iter], sizes[iter], sizes[iter+1], sizes[iter+1]);
    insert_matrix(resultant, c21, sizes[iter+1], 0, sizes[iter], sizes[iter], sizes[iter+1], sizes[iter+1]);
    insert_matrix(resultant, c22, sizes[iter+1], sizes[iter+1], sizes[iter], sizes[iter], sizes[iter+1], sizes[iter+1]);

    FREE(c11);
    FREE(c12);
    FREE(c21);
    FREE(c22);
  } else if(contains(sources, MMS, root)) { //helper code
    MPI_Status status_a, status_b;
    MPI_Recv(recv_A, sizes[iter+1] * sizes[iter+1], MPI_FLOAT, root, 0, MPI_COMM_WORLD, &status_a);
    MPI_Recv(recv_B, sizes[iter+1] * sizes[iter+1], MPI_FLOAT, root, 0, MPI_COMM_WORLD, &status_b);
    strassen_mult_helper(size, root, iter+1);
    int a_count, b_count;
    MPI_Get_count(&status_a, MPI_FLOAT, &a_count);
    MPI_Get_count(&status_b, MPI_FLOAT, &b_count);
    printf("%d, %d\n", a_count, b_count);
    
    float* m_mat;
    for(i = 0; i < MMS; ++i) {
      if(other_sources[i] >= world_rank) {
        break;
      } else {
        strassen_mult_helper(size, other_sources[i], iter+1);
      }
    }
    m_mat = strassen_mult(recv_A, recv_B, size, world_rank, iter+1);
    MPI_Send(m_mat, sizes[iter+1] * sizes[iter+1], MPI_FLOAT, root, 1, MPI_COMM_WORLD);
    for(; i < MMS; ++i) {
      if(other_sources[i] == world_rank) continue;
      strassen_mult_helper(size, other_sources[i], iter+1);
    }
  } else { //helper code when proc is unecessary
    for(i = 0; i < MMS; ++i) {
      if(other_sources[i] == world_rank) continue;
        strassen_mult_helper(size, other_sources[i], iter+1);
    }
  }
  FREE(recv_A);
  FREE(recv_B);
  for(i = 0; i < MMS; ++i) {
    FREE(m_mats[i]);
    FREE(sub_As[i]);
    FREE(sub_Bs[i]);
  }
  FREE(m_mats);
  FREE(sub_As);
  FREE(sub_Bs);
  printf("%d, %d returning %d\n", world_rank, root, iter);
  return resultant;
}

void strassen_mult_helper(int size, int root, int iter) {
  if(iter >= 4)
    return;
  strassen_mult(NULL, NULL, size, root, iter);
}

void strassen_extract(float *mat_A, float *mat_B, int rows, int cols, float** sub_As, float** sub_Bs) {
  //printf("%d, %d\n", rows, cols);
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
      //printf("%f\n", a_quad[i * 2 +j][0]);
    }
  }
  matrix_sum(sub_As[0], a_quad[0], a_quad[3], rows/2, cols/2);
  matrix_sum(sub_As[1], a_quad[2], a_quad[3], rows/2, cols/2);
  memcpy(sub_As[2], a_quad[1], rows * cols / 4 * sizeof(float));
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
  //printf("%f\n", sub_As[0][0]);
  for(i = 0; i < 4; ++i) {
    FREE(a_quad[i]);
    FREE(b_quad[i]);
  }
  FREE(a_quad);
  FREE(b_quad);
}
/*float *l1A, *l1B, **l2A, **l2A, **l3A, **l3B;
  mmalloc((void**)&l1A, sizes[1] * sizes[1] * sizeof(float));
  mmalloc((void**)&l1B, sizes[1] * sizes[1] * sizeof(float));
  mmalloc((void**)&l2A, MMS * sizeof(float*));
  mmalloc((void**)&l2B, MMS * sizeof(float*));
  mmalloc((void**)&l3A, MMS * MMS * sizeof(float*));
  mmalloc((void**)&l3B, MMS * MMS * sizeof(float*));
  for(i = 0; i < MMS; ++i) {
    mmalloc((void**)&l2A[i], sizes[2] * sizes[2] * sizeof(float));
    mmalloc((void**)&l2B[i], sizes[2] * sizes[2] * sizeof(float));
  }
  for(i = 0; i < MMS*MMS; ++i) {
    mmalloc((void**)&l3A[i], sizes[3] * sizes[3] * sizeof(float));
    mmalloc((void**)&l3B[i], sizes[3] * sizes[3] * sizeof(float));
  }
  if(world_rank == 0) {
    for(i = 1; i < MMS; ++i) {
      MPI_Send(sub_As[i], sizes[1] * sizes[1], MPI_FLOAT, i, 0, MPI_COMM_WORLD);
      MPI_Send(sub_Bs[i], sizes[1] * sizes[1], MPI_FLOAT, i, 0, MPI_COMM_WORLD);
    }
    memcpy(l1A, sub_As[0], sizes[1] * sizes[1] * sizeof(float));
    memcpy(l1B, sub_Bs[0], sizes[1] * sizes[1] * sizeof(float));
  }

  float *send_buf, *A_buf, *B_buf;
  int cont = 1;
  int skipped = 0;
  int recv_size;
  MPI_Status status;
  while(cont) {
    if(world_rank > 0 || skipped) {
      j = 0;
      while(sources[j] != world_rank) {
        MPI_Probe(sources[j], 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_FLOAT, &recv_size);
        mmalloc((void**)&A_buf, recv_size * sizeof(float));
        mmalloc((void**)&B_buf, recv_size * sizeof(float));
        MPI_Recv(A_buf, recv_size, MPI_FLOAT, sources[j], 

        MPI_Probe(sources[j], 0, MPI_COMM_WORLD, &status);

    } else {
      skipped = 1;
    }

  }*/


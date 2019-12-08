#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "matrix.h"
#include "mpi_matrix.h"
#include "strassens.h"
#define MMS 7
#define LEAVES 2401
#define BASE_TAG 0x00FFFFFF
#define ELEMENT_TAG 0xFF000000

float* strassen_mult(float* mat_A, float* mat_B, int rows, int cols) {
  int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm upstream_comm;
  MPI_Comm_split(MPI_COMM_WORLD, 0, world_rank, &upstream_comm);
  MPI_Status status;
  MPI_Request send_request;
  //int status_size = 10; //keeps track of internel size of array
  //int status_length = 0;
  //statuses = malloc(status_size * sizeof(MPI_Status));
  //int status_size_1 = 10; //keeps track of internel size of array
  //int status_length_1 = 0;
  //statuses_1 = malloc(status_size_1 * sizeof(MPI_Status));

  const int size_1 = rows / 2; //at this level tag is always 0
  const int size_2 = size_1 / 2; //at this level tag is world_rank_1+1
  const int size_3 = size_2 / 2; //at this level tag is world_rank_1+1<<7 + world_rank_2+1
  const int size_4 = size_3 / 2; //at this level tag is world_rank_1+1<<14 + world_rank_2+1<<7 + world_rank_3+1
  const int sizes[] = {rows, size_1, size_2, size_3, size_4};
  const int base_rank = world_rank * 7 % world_size;


  float *resultant, **sub_As, **sub_Bs, *send_buf, *recv_buf;
  void* msg_buffer;
  int *destinations = malloc(MMS * sizeof(int));
  int i, j;
  int recv_size;
  int leaf_count = 0;
  int flag = 0;
  int buf_length = 0, temp;

  MPI_Pack_size(size_1 * size_1 * 2, MPI_FLOAT, MPI_COMM_WORLD, &temp);
  buf_length += temp;
  MPI_Pack_size(size_2 * size_2 * 2, MPI_FLOAT, MPI_COMM_WORLD, &temp);
  buf_length += temp * 7;
  MPI_Pack_size(size_3 * size_3 * 2, MPI_FLOAT, MPI_COMM_WORLD, &temp);
  buf_length += temp * 49;
  MPI_Pack_size(size_3 * size_3 * 2, MPI_FLOAT, MPI_COMM_WORLD, &temp);
  buf_length += temp * 7 * 7 * 7;
  MPI_Pack_size(size_4 * size_3 * 2, MPI_FLOAT, MPI_COMM_WORLD, &temp);
  buf_length += temp * 7 * 7 * 7 * 7;
  MPI_Pack_size(size_3 * size_3, MPI_FLOAT, upstream_comm, &temp);
  buf_length += temp * 7 * 7 * 7;
  MPI_Pack_size(size_2 * size_2, MPI_FLOAT, upstream_comm, &temp);
  buf_length += temp * 7 * 7;
  MPI_Pack_size(size_1 * size_1, MPI_FLOAT, upstream_comm, &temp);
  buf_length += temp * 7;
  MPI_Pack_size(1, MPI_FLOAT, MPI_COMM_WORLD, &temp);
  buf_length += temp * LEAVES;
  buf_length += LEAVES * 7 * MPI_BSEND_OVERHEAD;

  resultant = NULL;

  for(i = 0; i < MMS; ++i) {
    destinations[i] = (base_rank + i) % world_size;
  }

  mmalloc((void**)&msg_buffer, buf_length);
  i = MPI_Buffer_attach(msg_buffer, buf_length);
  printf("%d\n", i);

  if(world_rank == 0) {
    mmalloc((void**)&sub_As, MMS * sizeof(float*));
    mmalloc((void**)&sub_Bs, MMS * sizeof(float*));
    for(i = 0; i < MMS; ++i) {
      mmalloc((void**)&sub_As[i], size_1 * size_1 * sizeof(float));
      mmalloc((void**)&sub_Bs[i], size_1 * size_1 * sizeof(float));
    }
    mmalloc((void**)&resultant, rows * cols * sizeof(float));
    printf("extracting initial set\n");
    strassen_extract(mat_A, mat_B, rows, cols, sub_As, sub_Bs);
    mmalloc((void**)&send_buf, size_1 * size_1 * 2);
    printf("Sending initial set\n");
    for(i = 0; i < MMS; ++i) { 
      memcpy(send_buf, sub_As[i], size_1 * size_1 * sizeof(float));
      memcpy(send_buf + size_1 * size_1, sub_Bs[i], size_1 * size_1 * sizeof(float));
      printf("time to send\n");
      MPI_Bsend(send_buf, size_1 * size_1 * 2, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
      //MPI_Ibsend(send_buf, size_1 * size_1 * 2, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &send_request);
      //while(!flag)
          //MPI_Test(&send_request, &flag, MPI_STATUS_IGNORE);
      //flag = 0;
    }
    printf("Sent initial set\n");
    for(i = 0; i < MMS; ++i) {
      FREE(sub_As[i]);
      FREE(sub_Bs[i]);
    }
    FREE(sub_As);
    FREE(sub_Bs);
    //FREE(send_buf);
  }
    if(world_rank == 0) {
        printf("msg time\n");
    }

  int cont = 1;
  while(cont) {
    float *buf_A, *buf_B;
    //keep spliting until at right level

    if(world_rank == 0) {
        printf("msg probing\n");
    }
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    /*if(status_length == status_size) {
      statuses = realloc(statuses, status_size * 2 * sizeof(MPI_Status));
      status_size *= 2;
    }*/
    //gets the length of the 
    MPI_Get_count(&status, MPI_FLOAT, &recv_size);

    mmalloc((void**)&recv_buf, recv_size * sizeof(float));

    MPI_Recv(recv_buf, recv_size, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(world_rank == 0) {
        printf("msg got, size: %d\n", recv_size);
    }

    buf_A = recv_buf; //A is stored in the first half
    buf_B = recv_buf + (recv_size / 2); //B is stored in the second half

    if(recv_size / 2 == size_4 * size_4) {
      //local mult and ISend up and send msg 1 to root
      send_buf = matrix_mult(buf_A, size_1, size_1, buf_B, size_1, size_1);
      int tag = status.MPI_TAG >> 7;
      int target = status.MPI_SOURCE;
      if(world_rank == 0)
        printf("%d local mult, %d\n", world_rank, target);
      float msg = 1;
      //MPI_Ibsend(send_buf, size_4 * size_4, MPI_FLOAT, target, tag, upstream_comm, &send_request);
      MPI_Bsend(send_buf, size_4 * size_4, MPI_FLOAT, target, tag, MPI_COMM_WORLD);
      //MPI_Ibsend(&msg, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &send_request);
      MPI_Bsend(&msg, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
      FREE(send_buf);
    } else if(recv_size > 1) {
      //divide up the A and B matrices and ISend out
      float **sub_As, **sub_Bs;
      mmalloc((void**)&sub_As, MMS * sizeof(float*));
      mmalloc((void**)&sub_Bs, MMS * sizeof(float*));
      int size = 0;
      for(i = 0; i < 5; ++i) {
        if(recv_size == sizes[i] * sizes[i] * 2) size = sizes[i];
      }
      for(i = 0; i < MMS; ++i) {
        mmalloc((void**)&sub_As[i], size * size / 4 * sizeof(float));
        mmalloc((void**)&sub_Bs[i], size * size / 4 * sizeof(float));
      }
      strassen_extract(buf_A, buf_B, size, size, sub_As, sub_Bs);
      mmalloc((void**)&send_buf, size * size / 2);
      //figure out how to best distribute the matrices to minimize overlap
      //tag = curr_tag<<7 + world_rank + 1
      int tag = (status.MPI_TAG << 7) | (world_rank + 1);
      for(i = 0; i < MMS; ++i) {
        memcpy(send_buf, sub_As[i], size * size / 4 * sizeof(float));
        memcpy(send_buf + size * size / 4, sub_Bs[i], size * size / 4 * sizeof(float));
        //Sends out the a and b matrices along with an encoding of the path and the number of the M matrix

        if(world_rank == 0) {
            printf("sending out the goods: %d to %d\n", size * size / 2, destinations[i]);
        }
        //MPI_Ibsend(send_buf, size * size / 2, MPI_FLOAT, destinations[i], tag, MPI_COMM_WORLD, &send_request);
        MPI_Bsend(send_buf, size * size / 2, MPI_FLOAT, destinations[i], tag, MPI_COMM_WORLD);
        /*while(!flag)
          MPI_Test(&send_request, &flag, MPI_STATUS_IGNORE);
        flag = 0;*/
      }

      printf("goods sent\n");
      FREE(send_buf);
      for(i = 0; i < MMS; ++i) {
        FREE(sub_As[i]);
        FREE(sub_Bs[i]);
      }
      FREE(sub_As);
      FREE(sub_Bs);
    } else {
      if(world_rank == 0)  {
        leaf_count++;
        //Send out signal to start upstream processes
        if(leaf_count >= LEAVES) {
          float msg = 1;
          for(i = 1; i < world_size; ++i) {
            //MPI_Ibsend(&msg, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &send_request);
            MPI_Bsend(&msg, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
          }
          cont = 0;
        }
      } else {
        cont = 0;
      }
    }
    FREE(recv_buf);
  }

  cont = 1;
  if(world_rank == 0)
      printf("upstream time\n");

  while(cont) {
    //upstream loop
    //have one look for any tag, then get size more with the same tag
    //gives us a way to bcast an end message to bust on out of here
    
    float **buf_Ms, *subresultant, **quads; 
    int curr_tag, first_sender, dest_offset;
    //keep spliting until at right level
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, upstream_comm, &status);
    
    /*if(status_length_1 == status_size_1) {
      statuses_1 = realloc(statuses_1, status_size_1 * 2 * sizeof(MPI_Status));
      status_size_1 *= 2;
    }*/
    //gets the length of the 
    MPI_Get_count(&status, MPI_FLOAT, &recv_size);
    curr_tag = status.MPI_TAG;
    first_sender = status.MPI_SOURCE;

    mmalloc((void**)&recv_buf, recv_size * sizeof(float));
    mmalloc((void**)&buf_Ms, MMS * sizeof(float*));
    mmalloc((void**)&recv_buf, recv_size * sizeof(float));

    for(i = 0; i < MMS; ++i) {
      mmalloc((void**)&buf_Ms[i], recv_size * sizeof(float));
      if(destinations[i] == first_sender) dest_offset = i;
    }

    MPI_Recv(recv_buf, recv_size, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, upstream_comm, &status);
    if(world_rank == 0)
      printf("upstream msg, %d\n", recv_size);
    if(recv_size > 1) {
      memcpy(buf_Ms[dest_offset], recv_buf, recv_size * sizeof(float));
      for(i = 1; i < 7; ++i) {
        //look for the six messages corresponding to the six other matrices that should be coming in with the given tag
        MPI_Recv(buf_Ms[(dest_offset + i)%MMS], recv_size, MPI_FLOAT, destinations[(dest_offset + i)%MMS], curr_tag, upstream_comm, MPI_STATUS_IGNORE);
      }

      //combine the M matrices into a sub result and send back up
      int target = (curr_tag == 0) ? 0 : (curr_tag & 0x7F ) - 1;
      if(world_rank == 0)
        printf("upstream msg more parts, %x\n", curr_tag);
      //target = (target < 0) ? 0 : target;
      curr_tag = curr_tag >> 7;
      mmalloc((void**)&subresultant, recv_size * 4 * sizeof(float));
      mmalloc((void**)&quads, 4 * sizeof(float*));
      for(i = 0; i < 4; ++i) {
        mmalloc((void**)&quads[i], recv_size * sizeof(float));
      }

      int size = 0;
      for(i = 0; i < 5; ++i) {
        if(recv_size == sizes[i] * sizes[i]) size = sizes[i];
      }

      //C11
      matrix_sum(quads[0], buf_Ms[1], buf_Ms[3], size, size);
      matrix_diff(quads[0], quads[0], buf_Ms[4], size, size);
      matrix_sum(quads[0], quads[0], buf_Ms[6], size, size);

      //C12
      matrix_sum(quads[1], buf_Ms[2], buf_Ms[4], size, size);

      //C21
      matrix_sum(quads[2], buf_Ms[1], buf_Ms[3], size, size);

      //C22
      matrix_diff(quads[3], buf_Ms[0], buf_Ms[1], size, size);
      matrix_sum(quads[3], quads[3], buf_Ms[2], size, size);
      matrix_sum(quads[3], quads[3], buf_Ms[5], size, size);


      insert_matrix(subresultant, quads[0], 0, 0, size*2, size*2, size, size); 
      insert_matrix(subresultant, quads[1], 0, size, size*2, size*2, size, size); 
      insert_matrix(subresultant, quads[2], size, 0, size*2, size*2, size, size); 
      insert_matrix(subresultant, quads[3], size, size, size*2, size*2, size, size); 
      
      if(size * size * 4 == rows * cols) {
        memcpy(resultant, subresultant, rows * cols * sizeof(float));
        cont = 0;
        float msg = 1;
        for(i = 1; i < world_size; ++i) {
          //MPI_Ibsend(&msg, 1, MPI_FLOAT, i, 0, upstream_comm, &send_request);
          MPI_Bsend(&msg, 1, MPI_FLOAT, i, 0, upstream_comm);
        }
      } else {
        //MPI_Ibsend(subresultant, size * size * 4, MPI_FLOAT, target, curr_tag, upstream_comm, &send_request);
        MPI_Bsend(subresultant, size * size * 4, MPI_FLOAT, target, curr_tag, upstream_comm);
      }

      for(i = 0; i < 4; ++i) {
        FREE(quads[i]);
      }
      FREE(quads);
      FREE(subresultant);
    } else {
      cont = 0;
    }

    for(i = 0; i < MMS; ++i) {
      FREE(buf_Ms[i]);
    }
    FREE(buf_Ms);
    FREE(recv_buf);
  }
  FREE(destinations);
  return resultant;
}

void strassen_mult_helper(int rows, int cols) {
  strassen_mult(NULL, NULL, rows, cols);

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
  for(i = 0; i < 4; ++i) {
    FREE(a_quad[i]);
    FREE(b_quad[i]);
  }
  FREE(a_quad);
  FREE(b_quad);
}

/*
//loop, send out one at a time, unless iter == 3 then locally multiply and send up
  //Maybe split by groups of seven processors
  //Maybe divide up into the 3rd level of recursion, send out, work back up from there
  if(iter == 0) {
    //send out all seven
    if(world_rank == 0) {
      //send out
      for(i = 1; i < MMS; ++i) {
        MPI_Send(sub_As[i], rows * cols / 16, MPI_FLOAT,  i, 0, MPI_COMM_WORLD);
        MPI_Send(sub_Bs[i], rows * cols / 16, MPI_FLOAT,  i, 0, MPI_COMM_WORLD);
      }
    } else if(world_rank < MMS){
      //receive
      MPI_Recv(sub_As[0], rows * cols / 16, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(sub_Bs[0], rows * cols / 16, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if(world_rank < MMS) {
      sub_resultant[0] = strassen_mult(sub_As[0], sub_Bs[0], rows/4, cols/4, 1, group);
    } else {
      strassen_mult_helper(NULL, NULL, rows/4, cols/4, 1, 0);
    }
  } else if(iter == 1) {
    //split up into groups if possible up to seven groups
    //divide into groups then go down tree and calc
    if(world_rank < MMS) {
      for(i = 0; i < ; ++i) {

      }
    } else {
      
    }
  } else if(iter < 3) {
    for(j = 0; j < MMS; ++j) {
      for(i = 0; i < MMS; ++i) {
        if(i + root == world_rank) continue;
        MPI_Send(sub_As, rows * cols / 16, MPI_FLOAT, (world_rank + i) % world_size, 0, MPI_COMM_WORLD);
      }
    }
  } else {
    //Local multiply
  }*/

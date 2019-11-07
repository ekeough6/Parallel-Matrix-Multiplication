#ifndef MPI_MATRIX_H
#define MPI_MATRIX_H

float* roll_rows_base(float* row, int turn, int rA, int cA, MPI_Comm communicator);

int last_row_len(int n, int world_size);

int first_rows_len(int n, int world_size);

void combineResults(void* in, void* inout, int *len, MPI_Datatype *dptr);
#endif

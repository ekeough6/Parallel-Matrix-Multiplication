#ifndef RING_H
#define RING_H

//multiples mat_A and mat_B in parallel into resultant using the ring method
float* ring_mult(float* mat_A, int rA, int cA, float* mat_B, int rB, int cB);

//function to be called by processors that don't cantain the matrix
void ring_mult_helper(int rA, int cA, int rB, int cB);

//rolls up the rows as needed for the ring method
float* roll_rows(float* row, int turn, int rA, int cA);


#endif 

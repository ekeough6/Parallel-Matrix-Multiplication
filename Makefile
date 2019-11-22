CC=gcc
NVCC=nvcc
MPICC=mpicc
CFLAGS=-I./inc -lm 
NVCFLAGS=-I./inc -c
NVCFLAGS2=-dlink
CUDAFLAG=-lcudart
TESTSOURCES=src/tester.c src/matrix.c
RINGSOURCES=src/ring_test.c src/ring.c src/matrix.c src/mpi_matrix.c
BMRSOURCES=src/bmr.c src/matrix.c src/mpi_matrix.c src/bmr_test.c
CUDASOURCES=src/gpu_matrix.cu
TESTOBJECTS=$(TESTSOURCES:.c=.o)
RINGOBJECTS=$(RINGSOURCES:.c=.o)
BMROBJECTS=$(BMRSOURCES:.c=.o)
CUDAOBJECTS=$(CUDASOURCES:.cu=.o)
CUDAMATRIX=cuda_matrix.o
TESTEXEC=test
RINGEXEC=ring
BMREXEC=bmr
EXECS=$(TESTEXEC) $(RINGEXEC) $(BMREXEC)

$(TESTEXEC): $(TESTSOURCES)
	$(CC) $(CFLAGS) $(TESTSOURCES) -o $@ -g

$(RINGEXEC): $(RINGSOURCES)
	$(MPICC) $(CFLAGS) $(RINGSOURCES) -o $@

$(BMREXEC): $(BMRSOURCES)
	$(MPICC) $(CFLAGS) $(BMRSOURCES) -o $@ -lm

$(TESTEXEC)-gpu: $(TESTSOURCES)
	$(NVCC) $(NVCFLAGS) $(CUDASOURCES) -o $(CUDAOBJECTS)
	$(CC) $(CFLAGS) -D USE_GPU -c src/tester.c  -o tester.o
	$(CC) $(CFLAGS) -D USE_GPU -c src/matrix.c  -o matrix.o
	$(CC) $(CUDAOBJECTS) tester.o matrix.o $(CUDAFLAG) -o $@

$(RINGEXEC)-gpu: $(RINGSOURCES)
	$(NVCC) $(NVCFLAGS) $(CUDASOURCES) -o $(CUDAOBJECTS)
	$(CC) $(CFLAGS) -D USE_GPU -c src/matrix.c -o matrix.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/ring.c -o ring.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/ring_test.c -o ring_test.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/mpi_matrix.c -o mpi_matrix.o
	$(MPICC) $(CUDAOBJECTS) mpi_matrix.o matrix.o ring.o ring_test.o $(CUDAFLAG) -o $@ 
	rm *.o

$(BMREXEC)-gpu: $(BMRSOURCES)
	$(NVCC) $(NVCFLAGS) $(CUDASOURCES) -o $(CUDAOBJECTS)
	$(CC) $(CFLAGS) -D USE_GPU -c src/matrix.c -o matrix.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/bmr.c -o bmr.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/bmr_test.c -o bmr_test.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/mpi_matrix.c -o mpi_matrix.o
	$(MPICC) $(CUDAOBJECTS) mpi_matrix.o matrix.o bmr.o bmr_test.o $(CUDAFLAG) -lm -o $@ 
	rm *.o

clean:
	rm $(EXECS) test-gpu ring-gpu

all:


CC=gcc
NVCC=nvcc
MPICC=mpicc
CFLAGS=-I./inc -lm 
NVCFLAGS=-I./inc -c
NVCFLAGS2=-dlink
CUDAFLAG=-lcudart
TESTSOURCES=src/tester.c src/matrix.c
GENSOURCES=src/matrix_gen.c src/matrix.c
SGENSOURCES=src/matrix_gen_strassen.c src/matrix.c
COMPSOURCES=src/matrix_compare.c src/matrix.c
RINGSOURCES=src/ring_test.c src/ring.c src/matrix.c src/mpi_matrix.c
BMRSOURCES=src/bmr.c src/matrix.c src/mpi_matrix.c src/bmr_test.c
STRASSENSOURCES=src/strassens.c src/matrix.c src/mpi_matrix.c src/strassens_test.c
CUDASOURCES=src/gpu_matrix.cu
TESTOBJECTS=$(TESTSOURCES:.c=.o)
GENOBJECTS=$(GENSOURCES:.c=.o)
COMPOBJECTS=$(COMPSOURCES:.c=.o)
RINGOBJECTS=$(RINGSOURCES:.c=.o)
BMROBJECTS=$(BMRSOURCES:.c=.o)
CUDAOBJECTS=$(CUDASOURCES:.cu=.o)
CUDAMATRIX=cuda_matrix.o
TESTEXEC=test
GENEXEC=mat_gen
SGENEXEC=mat_gen_strassen
COMPEXEC=mat_comp
RINGEXEC=ring
BMREXEC=bmr
STRASSENEXEC=strassens
EXECS=$(TESTEXEC) $(RINGEXEC) $(BMREXEC) $(GENEXEC) $(COMPEXEC) $(STRASSENEXEC)

$(TESTEXEC): $(TESTSOURCES)
	$(CC) $(CFLAGS) $(TESTSOURCES) -o $@

$(GENEXEC): $(GENSOURCES)
	$(CC) $(CFLAGS) $(GENSOURCES) -o $@

$(SGENEXEC): $(SGENSOURCES)
	$(CC) $(CFLAGS) $(SGENSOURCES) -o $@

$(COMPEXEC): $(COMPSOURCES)
	$(CC) $(CFLAGS) $(COMPSOURCES) -o $@

$(RINGEXEC): $(RINGSOURCES)
	$(MPICC) $(CFLAGS) $(RINGSOURCES) -o $@

$(BMREXEC): $(BMRSOURCES)
	$(MPICC) $(CFLAGS) $(BMRSOURCES) -o $@ -lm

$(STRASSENEXEC): $(STRASSENSOURCES)
	$(MPICC) $(CFLAGS) $(STRASSENSOURCES) -o $@ -lm -g

$(TESTEXEC)-gpu: $(TESTSOURCES)
	$(NVCC) $(NVCFLAGS) $(CUDASOURCES) -o $(CUDAOBJECTS)
	$(CC) $(CFLAGS) -D USE_GPU -c src/tester.c  -o tester.o
	$(CC) $(CFLAGS) -D USE_GPU -c src/matrix.c  -o matrix.o
	$(CC) $(CUDAOBJECTS) tester.o matrix.o $(CUDAFLAG) -o $@
	rm *.o

$(GENEXEC)-gpu: $(GPUSOURCES)
	$(NVCC) $(NVCFLAGS) $(CUDASOURCES) -o $(CUDAOBJECTS)
	$(CC) $(CFLAGS) -D USE_GPU -c src/matrix_gen.c  -o mat_gen.o
	$(CC) $(CFLAGS) -D USE_GPU -c src/matrix.c  -o matrix.o
	$(CC) $(CUDAOBJECTS) mat_gen.o matrix.o $(CUDAFLAG) -o $@
	rm *.o

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

$(STRASSENEXEC)-gpu: $(STRASSENSOURCES)
	$(NVCC) $(NVCFLAGS) $(CUDASOURCES) -o $(CUDAOBJECTS)
	$(CC) $(CFLAGS) -D USE_GPU -c src/matrix.c -o matrix.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/strassens.c -o strassens.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/strassens_test.c -o strassens_test.o
	$(MPICC) $(CFLAGS) -D USE_GPU -c src/mpi_matrix.c -o mpi_matrix.o
	$(MPICC) $(CUDAOBJECTS) mpi_matrix.o matrix.o strassens.o strassens_test.o $(CUDAFLAG) -lm -o $@ 
	rm *.o


clean:
	rm $(EXECS) test-gpu ring-gpu bmr-gpu mat_gen-gpu 

all:


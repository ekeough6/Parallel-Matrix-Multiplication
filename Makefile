CC=gcc
MPICC=mpicc
CFLAGS=-I./inc -lm
TESTSOURCES=src/tester.c src/matrix.c
RINGSOURCES=src/ring_test.c src/ring.c src/matrix.c src/mpi_matrix.c
BMRSOURCES=src/bmr.c src/matrix.c src/mpi_matrix.c src/bmr_test.c
TESTOBJECTS=$(TESTSOURCES:.c=.o)
RINGOBJECTS=$(RINGSOURCES:.c=.o)
BMROBJECTS=$(BMRSOURCES:.c=.o)
TESTEXEC=test
RINGEXEC=ring
BMREXEC=bmr
EXECS=$(TESTEXEC) $(RINGEXEC) $(BMREXEC)

$(TESTEXEC): $(TESTSOURCES)
	$(CC) $(CFLAGS) $(TESTSOURCES) -o $@

$(RINGEXEC): $(RINGSOURCES)
	$(MPICC) $(CFLAGS) $(RINGSOURCES) -o $@

$(BMREXEC): $(BMRSOURCES)
	$(MPICC) $(CFLAGS) $(BMRSOURCES) -o $@ -lm
clean:
	rm $(EXECS)
all:


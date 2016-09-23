RES=-DPRINT_RESULTS
CFLAGS=-O3 -lm -Wall
CC=mpicc
CONV=-DTEST_CONV

jacobi_parallel: jacobi_parallel.o utils.o
	$(CC) $(CFLAGS) $(RES) $(CONV) jacobi_parallel.o utils.o -o jacobi_parallel
utils.o: utils.c
	gcc -c  $(CFLAGS) $(RES) $(CONV) utils.c
jacobi_parallel.o: jacobi_parallel.c
	$(CC) -c  $(CFLAGS) $(RES) $(CONV) jacobi_parallel.c







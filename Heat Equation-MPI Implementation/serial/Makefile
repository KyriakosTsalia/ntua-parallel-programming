CC=gcc
CFLAGS= -lm -Wall
RES=-DPRINT_RESULTS
CONV=-DTEST_CONV


aall: jacobi seidelsor redblacksor


jacobi: Jacobi_serial.c utils.c
	$(CC) $(CFLAGS) $(RES) $(CONV) Jacobi_serial.c utils.c -o jacobi

seidelsor: GaussSeidelSOR_serial.c utils.c
	$(CC) $(CFLAGS) $(RES) $(CONV) GaussSeidelSOR_serial.c utils.c -o seidelsor

redblacksor: RedBlackSOR_serial.c utils.c 
	$(CC) $(CFLAGS) $(RES) $(CONV) RedBlackSOR_serial.c utils.c -o redblacksor

clean:
	rm jacobi seidelsor redblacksor


.phony: all clean

all: fw_sr_CILK  fw_tiled_CILK fw_tiled_omp fw_omp 

CC=gcc
CFLAGS= -Wall -O3 -Wno-unused-variable

HDEPS+=%.h

OBJS=util.o

fw_tiled_omp:fw_tiled_omp.c
	gcc -fopenmp $(OBJS) fw_tiled_omp.c -o fw_tiled_omp $(FLAGS)
fw_tiled_CILK:fw_tiled.cilk
	cilkc $(OBJS) fw_tiled.cilk -o fw_tiled_CILK $(FLAGS)
fw_sr_CILK:fw_sr.cilk
	cilkc $(OBJS) fw_sr.cilk -o fw_sr_CILK $(CFLAGS)
fw_omp:fw.c
	gcc -fopenmp $(OBJS) fw.c -o fw_omp $(CFLAGS)
%.o: %.c $(HDEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o fw_sr 


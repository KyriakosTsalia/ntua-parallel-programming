#!/bin/bash

#PBS -o fw_cilk.out
#PBS -e fw_cilk.err


cd a3/fw_parallel/
module load cilk-mit

for N in 1024 2048 4096
do
for i in 1 2 4 8 16 32 64
do

#PBS -l ppn=$i

./fw_tiled_CILK --nproc $i $N 64 >>Results3
done
done



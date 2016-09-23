#!/bin/bash 

#PBS -o fw_omp.out
#PBS -e fw_omp.err


cd a3/fw_parallel/
module load openmp

for N in 1024 2048 4096 
do
for i in 1 2 4 8 16 32 64 
do

#PBS -l ppn=$i

export OMP_NUM_THREADS=$i
./fw_omp $N >>Results1
done 
done

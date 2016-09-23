#!/bin/bash

## Give the Job a descriptive name
#PBS -N jacobi

## Output and error files
#PBS -o jacobi.out
#PBS -e jacobi.err

## Limit memory, runtime etc.
#PBS -l walltime=01:00:00

## How many nodes:processors_per_node should we get?
#PBS -l nodes=3:ppn=4

## Start 
##echo "PBS_NODEFILE = $PBS_NODEFILE"
##cat $PBS_NODEFILE

## Run the job (use full paths to make sure we execute the correct thing) 
## NOTE: Fix the path to show to your executable! 

x=$3
y=$4
procs=$(( x * y ))
mpirun -x MX_RCACHE=8 --mca btl tcp,self -np $procs --bynode ./jacobi_parallel $1 $2 $3 $4

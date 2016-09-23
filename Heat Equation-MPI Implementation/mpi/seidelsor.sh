#!/bin/bash

## Give the Job a descriptive name
#PBS -N seidelsor

## Output and error files
#PBS -o seidelsor.out
#PBS -e seidelsor.err

## Limit memory, runtime etc.
#PBS -l walltime=01:00:00

## How many nodes:processors_per_node should we get?
#PBS -l nodes=3:ppn=4

## Start 
##echo "PBS_NODEFILE = $PBS_NODEFILE"
##cat $PBS_NODEFILE
x=$3
y=$4
procs=$(( x * y ))
mpirun -x MX_RCACHE=8 --mca btl tcp,self -np $procs --bynode ./seidelsor_parallel $1 $2 $3 $4


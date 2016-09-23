#!/bin/bash

#PBS -o make.out
#PBS -e make.err


module load cilk-mit

cd a3/fw_parallel/
make


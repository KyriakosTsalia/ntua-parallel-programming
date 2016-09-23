#!/bin/bash 

#PBS -o z2.out
#PBS -e z2.err


cd a4/z2
for i in ttas_lock.c 
do
export LOCK_FILE=$i
rm linked_list
make
for j in 16 1024 8192
do
echo $i >> z2_results_${j} 

MT_CONF=0 ./linked_list $j >> z2_results_${j}
MT_CONF=0,1 ./linked_list $j >> z2_results_${j}
MT_CONF=0,1,2,3 ./linked_list $j >> z2_results_${j}
MT_CONF=0,1,2,3,4,5,6,7 ./linked_list $j >> z2_results_${j}
MT_CONF=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15  ./linked_list $j >> z2_results_${j}
MT_CONF=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 ./linked_list $j >> z2_results_${j}

done 
done






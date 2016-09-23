#!/bin/bash 

#PBS -o z3_20.out
#PBS -e z3_20.err


cd a4/z3
for i in ll_nb.c 
do
export LL_FILE=$i
rm linked_list
make
for j in 1024 8192
do
echo $i >> z3_results_${j}_20_40_40 

MT_CONF=0 ./linked_list $j 20 40 40  >> z3_results_${j}_20_40_40
MT_CONF=0,1 ./linked_list $j 20 40 40  >> z3_results_${j}_20_40_40
MT_CONF=0,1,2,3 ./linked_list $j 20 40 40  >> z3_results_${j}_20_40_40
MT_CONF=0,1,2,3,4,5,6,7 ./linked_list $j 20 40 40  >> z3_results_${j}_20_40_40
MT_CONF=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15  ./linked_list $j 20 40 40  >> z3_results_${j}_20_40_40
MT_CONF=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 ./linked_list $j 20 40 40  >> z3_results_${j}_20_40_40
MT_CONF=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 ./linked_list $j 20 40 40  >> z3_results_${j}_20_40_40

done 
done






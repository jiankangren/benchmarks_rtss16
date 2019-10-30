#!/bin/bash

# Run LU
for n in 512 1024 2048
do 
	for core in 1 3 6 9 12 15 18 21 24 27 30
	do
		~/codes/fss_rtss16_code/dag_benchmarks/lu/lu_omp $core n $n
	done
done
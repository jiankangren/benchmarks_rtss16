#!/bin/bash

# Run Cholesky
#for n in {1600..2900..100}
for n in 1500 3000
do 
	for core in 1 3 6 9 12 15 18 21 24 27 30
	do
		~/codes/fss_rtss16_code/dag_benchmarks/cholesky/ch_omp $core n $n
	done
done
#!/bin/bash

# Run Cholesky
for n in {1600..2900..100}
do 
	for core in 1
	do
		~/codes/fss_rtss16_code/dag_benchmarks/cholesky/ch_cilk_llvm $core n $n
	done
done
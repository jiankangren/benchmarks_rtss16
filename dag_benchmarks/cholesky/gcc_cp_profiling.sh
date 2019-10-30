#!/bin/bash

echo "START RUNNING CHOLESKY (Cilkprof + GCC)............"

# Run Cholesky
for n in {1600..2900..100}
do
	echo "====== Cholesky n = $n"
	~/codes/fss_rtss16_code/dag_benchmarks/cholesky/ch_cilk_cp_gcc_time 1 n $n
	echo
	echo "Burdened time:"
	~/codes/fss_rtss16_code/dag_benchmarks/cholesky/ch_cilk_cp_gcc_time_burdened 1 n $n
	echo
	echo
	
done
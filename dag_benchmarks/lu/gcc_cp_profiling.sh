#!/bin/bash

echo "START RUNNING LU (Cilkprof + GCC) ............"

# Run LU
for n in 128 256 512 1024 2048
do 
	echo "======== LU n = $n"
	~/codes/fss_rtss16_code/dag_benchmarks/lu/lu_cilk_cp_gcc_time 1 n $n
	echo

	echo "Burdened time:"
	~/codes/fss_rtss16_code/dag_benchmarks/lu/lu_cilk_cp_gcc_time_burdened 1 n $n
	echo
	echo
done
#!/bin/bash

echo "START RUNNING LU ............"

# Run LU
for n in 128 256 512 1024 2048
do
	echo "LU n = $n"
	~/cilktools-linux-004421/bin/cilkview ~/codes/fss_rtss16_code/dag_benchmarks/lu/lu_cilk_cv 1 n $n
	
	echo
	echo "GCC Cilkprof instruction count:"
	~/codes/fss_rtss16_code/dag_benchmarks/lu/lu_cilk_cp_gcc_ins 1 n $n
	echo
	echo
done
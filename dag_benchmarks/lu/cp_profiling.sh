#!/bin/bash

echo "START RUNNING LU (Cilkprof) ............"

# Run LU
for n in 128 256 512 1024 2048
do 
	echo "LU n = $n"
	~/codes/fss_rtss16_code/dag_benchmarks/lu/lu_cilk_cp 1 n $n
	echo
	echo
done
#!/bin/bash

echo "START RUNNING CHOLESKY (Cilkprof)............"

# Run Cholesky
for n in {1600..2900..100}
do
	echo "Cholesky n = $n"
	~/codes/fss_rtss16_code/dag_benchmarks/cholesky/ch_cilk_cp 1 n $n
	echo
	echo
	
done
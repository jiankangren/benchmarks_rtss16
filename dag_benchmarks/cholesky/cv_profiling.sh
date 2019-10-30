#!/bin/bash

echo "START RUNNING CHOLESKY ............"

# Run Cholesky
for n in {1600..2900..100}
do
	echo "Cholesky n = $n"
	~/cilktools-linux-004421/bin/cilkview ~/codes/fss_rtss16_code/dag_benchmarks/cholesky/ch_cilk_cv 1 n $n
	echo
	echo
	
done
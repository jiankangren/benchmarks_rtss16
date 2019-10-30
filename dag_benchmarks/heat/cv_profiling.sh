#!/bin/bash

echo "START RUNNING HEAT ............"

for nx in 512 1024 2048
do 
	for nt in 500 600 700 800 900 1000
	do
		echo "Heat nx = $nx, ny = $nx, nt = $nt"
		~/cilktools-linux-004421/bin/cilkview ~/codes/fss_rtss16_code/dag_benchmarks/heat/heat_cilk_cv 1 nx $nx ny $nx nt $nt
		echo
		echo
	done
done
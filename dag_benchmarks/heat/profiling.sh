#!/bin/bash

# Run Heat
# Use the same value for nx and ny
for nx in 512 1024 2048
do 
	for nt in 500 600 #700 800 900 1000
	do
		for core in 1 6 12 18 24 30 #3 6 9 12 15 18 21 24 27 30
		do
			#echo "$core nx $nx ny $nx nt $nt"
			~/codes/fss_rtss16_code/dag_benchmarks/heat/heat_cilk $core nx $nx ny $nx nt $nt
		done
	done
done
#!/bin/bash

# RUN CILK
#./proc_launch 1 ./heat_cilk_par 1 nx 4096 ny 1024 nt 800
./proc_launch 3 ./heat_cilk_par 3 nx 4096 ny 1024 nt 800
./proc_launch 6 ./heat_cilk_par 6 nx 4096 ny 1024 nt 800
./proc_launch 9 ./heat_cilk_par 9 nx 4096 ny 1024 nt 800
./proc_launch 12 ./heat_cilk_par 12 nx 4096 ny 1024 nt 800
./proc_launch 15 ./heat_cilk_par 15 nx 4096 ny 1024 nt 800
./proc_launch 18 ./heat_cilk_par 18 nx 4096 ny 1024 nt 800
./proc_launch 21 ./heat_cilk_par 21 nx 4096 ny 1024 nt 800
./proc_launch 24 ./heat_cilk_par 24 nx 4096 ny 1024 nt 800
./proc_launch 27 ./heat_cilk_par 27 nx 4096 ny 1024 nt 800
./proc_launch 30 ./heat_cilk_par 30 nx 4096 ny 1024 nt 800


# RUN OPENMP
./proc_launch 1 ./heat_omp_par 1 nx 4096 ny 1024 nt 800
./proc_launch 3 ./heat_omp_par 3 nx 4096 ny 1024 nt 800
./proc_launch 6 ./heat_omp_par 6 nx 4096 ny 1024 nt 800
./proc_launch 9 ./heat_omp_par 9 nx 4096 ny 1024 nt 800
./proc_launch 12 ./heat_omp_par 12 nx 4096 ny 1024 nt 800
./proc_launch 15 ./heat_omp_par 15 nx 4096 ny 1024 nt 800
./proc_launch 18 ./heat_omp_par 18 nx 4096 ny 1024 nt 800
./proc_launch 21 ./heat_omp_par 21 nx 4096 ny 1024 nt 800
./proc_launch 24 ./heat_omp_par 24 nx 4096 ny 1024 nt 800
./proc_launch 27 ./heat_omp_par 27 nx 4096 ny 1024 nt 800
./proc_launch 30 ./heat_omp_par 30 nx 4096 ny 1024 nt 800

#!/bin/bash

# RUN CILK
./proc_launch 3 ./lu_cilk_par 3 n 2048
./proc_launch 6 ./lu_cilk_par 6 n 2048
./proc_launch 9 ./lu_cilk_par 9 n 2048
./proc_launch 12 ./lu_cilk_par 12 n 2048
./proc_launch 15 ./lu_cilk_par 15 n 2048
./proc_launch 18 ./lu_cilk_par 18 n 2048
./proc_launch 21 ./lu_cilk_par 21 n 2048
./proc_launch 24 ./lu_cilk_par 24 n 2048
./proc_launch 27 ./lu_cilk_par 27 n 2048

# RUN OPENMP
./proc_launch 3 ./lu_omp_par 3 n 2048
./proc_launch 6 ./lu_omp_par 6 n 2048
./proc_launch 9 ./lu_omp_par 9 n 2048
./proc_launch 12 ./lu_omp_par 12 n 2048
./proc_launch 15 ./lu_omp_par 15 n 2048
./proc_launch 18 ./lu_omp_par 18 n 2048
./proc_launch 21 ./lu_omp_par 21 n 2048
./proc_launch 24 ./lu_omp_par 24 n 2048
./proc_launch 27 ./lu_omp_par 27 n 2048
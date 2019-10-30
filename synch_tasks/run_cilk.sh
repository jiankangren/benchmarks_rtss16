#!/bin/bash

# Run task type 3, only 100 times per point

#taskset -c 2 ./task_omp task_type$1.info results/type$1 1
#taskset -c 2-4 ./task_omp task_type$1.info results/type$1 3
#taskset -c 2-7 ./task_omp task_type$1.info results/type$1 6
#taskset -c 2-10 ./task_omp task_type$1.info results/type$1 9
#taskset -c 2-13 ./task_omp task_type$1.info results/type$1 12
#taskset -c 2-16 ./task_omp task_type$1.info results 15 1000
#taskset -c 2-19 ./task_omp task_type$1.info results 18 1000
#taskset -c 2-22 ./task_omp task_type$1.info results 21 1000
#taskset -c 2-25 ./task_omp task_type$1.info results 24 1000
#taskset -c 2-28 ./task_omp task_type$1.info results 27 1000
#taskset -c 2-31 ./task_omp task_type$1.info results 30 1000

taskset -c 0 ./task_cilk task_type$1.info results 1 900

taskset -c 0-2 ./task_cilk task_type$1.info results 3 900

taskset -c 0-5 ./task_cilk task_type$1.info results 6 900

taskset -c 0-4,8-11 ./task_cilk task_type$1.info results 9 900

taskset -c 0-5,8-13 ./task_cilk task_type$1.info results 12 900

taskset -c 0-7,8-14 ./task_cilk task_type$1.info results 15 900

taskset -c 0-5,8-13,16-21 ./task_cilk task_type$1.info results 18 900

taskset -c 0-6,8-14,16-22 ./task_cilk task_type$1.info results 21 900

taskset -c 0-7,8-15,16-23 ./task_cilk task_type$1.info results 24 900

taskset -c 0-6,8-14,16-22,24-29 ./task_cilk task_type$1.info results 27 900

taskset -c 0-7,8-15,16-23,24-29 ./task_cilk task_type$1.info results 30 900

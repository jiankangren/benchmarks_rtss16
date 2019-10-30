#!/bin/bash

# Run task type 1
taskset -c 2 ./task_cilk task_type$1.info results/type$1 1
taskset -c 2-4 ./task_cilk task_type$1.info results/type$1 3
taskset -c 2-7 ./task_cilk task_type$1.info results/type$1 6
taskset -c 2-10 ./task_cilk task_type$1.info results/type$1 9
taskset -c 2-13 ./task_cilk task_type$1.info results/type$1 12
taskset -c 2-16 ./task_cilk task_type$1.info results/type$1 15
taskset -c 2-19 ./task_cilk task_type$1.info results/type$1 18
taskset -c 2-22 ./task_cilk task_type$1.info results/type$1 21
taskset -c 2-25 ./task_cilk task_type$1.info results/type$1 24
taskset -c 2-28 ./task_cilk task_type$1.info results/type$1 27
taskset -c 2-31 ./task_cilk task_type$1.info results/type$1 30

taskset -c 2 ./task_omp task_type$1.info results/type$1 1
taskset -c 2-4 ./task_omp task_type$1.info results/type$1 3
taskset -c 2-7 ./task_omp task_type$1.info results/type$1 6
taskset -c 2-10 ./task_omp task_type$1.info results/type$1 9
taskset -c 2-13 ./task_omp task_type$1.info results/type$1 12
taskset -c 2-16 ./task_omp task_type$1.info results/type$1 15
taskset -c 2-19 ./task_omp task_type$1.info results/type$1 18
taskset -c 2-22 ./task_omp task_type$1.info results/type$1 21
taskset -c 2-25 ./task_omp task_type$1.info results/type$1 24
taskset -c 2-28 ./task_omp task_type$1.info results/type$1 27
taskset -c 2-31 ./task_omp task_type$1.info results/type$1 30
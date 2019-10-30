#!/bin/bash

# Run task type 3, only 100 times per point

#taskset -c 0 ./task_omp task_type$1.info results 1 1000

#taskset -c 0-2 ./task_omp task_type$1.info results 3 1000

#taskset -c 0-5 ./task_omp task_type$1.info results 6 1000

taskset -c 0-4,8-11 ./task_omp task_type$1.info results 9 1000

taskset -c 0-5,8-13 ./task_omp task_type$1.info results 12 1000

taskset -c 0-7,8-14 ./task_omp task_type$1.info results 15 1000

taskset -c 0-5,8-13,16-21 ./task_omp task_type$1.info results 18 1000

taskset -c 0-6,8-14,16-22 ./task_omp task_type$1.info results 21 1000

taskset -c 0-7,8-15,16-23 ./task_omp task_type$1.info results 24 1000

taskset -c 0-6,8-14,16-22,24-29 ./task_omp task_type$1.info results 27 1000

taskset -c 0-7,8-15,16-23,24-29 ./task_omp task_type$1.info results 30 1000

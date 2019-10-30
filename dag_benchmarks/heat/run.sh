#!/bin/bash

#ARGS="-nx 4096 -ny 512 -nt 500"
#ARGS="-nx 512 -ny 512 -nt 400"
#ARGS="-nx 512 -ny 512 -nt 500"
#ARGS="-nx 1024 -ny 512 -nt 300"

# Profiling the program with various inputs
#ARGS="-nx 4096 -ny 1024 -nt 800"
#ARGS="-nx 4096 -ny 1024 -nt 1000"
#ARGS="-nx 4096 -ny 512 -nt 3000"
#ARGS="-nx 4096 -ny 1024 -nt 4000"
ARGS="-nx 1024 -ny 4096 -nt 2000"


./heat_cilk 1 ${ARGS}
#./heat_omp 1 ${ARGS}

./heat_cilk 32 ${ARGS}
#./heat_omp 32 ${ARGS}

./heat_cilk 16 ${ARGS}
#./heat_omp 16 ${ARGS}

./heat_cilk 8 ${ARGS}
#./heat_omp 8 ${ARGS}

./heat_cilk 4 ${ARGS}
#./heat_omp 4 ${ARGS}

./heat_cilk 12 ${ARGS}
#./heat_omp 12 ${ARGS}

./heat_cilk 24 ${ARGS}
#./heat_omp 24 ${ARGS}

./heat_cilk 20 ${ARGS}
#./heat_omp 20 ${ARGS}

./heat_cilk 28 ${ARGS}
#./heat_omp 28 ${ARGS}
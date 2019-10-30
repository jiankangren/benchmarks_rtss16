#!/bin/bash

# Size of matrix must be power of 2 and multiple of 16
#ARGS="n 1024"
ARGS="n 2048"
#ARGS="n 4096"


./lu_cilk 1 ${ARGS}
#./lu_omp 1 ${ARGS}

./lu_cilk 32 ${ARGS}
#./lu_omp 32 ${ARGS}

./lu_cilk 16 ${ARGS}
#./lu_omp 16 ${ARGS}

./lu_cilk 8 ${ARGS}
#./lu_omp 8 ${ARGS}

./lu_cilk 4 ${ARGS}
#./lu_omp 4 ${ARGS}

./lu_cilk 12 ${ARGS}
#./lu_omp 12 ${ARGS}

./lu_cilk 24 ${ARGS}
#./lu_omp 24 ${ARGS}

./lu_cilk 20 ${ARGS}
#./lu_omp 20 ${ARGS}

./lu_cilk 28 ${ARGS}
#./lu_omp 28 ${ARGS}
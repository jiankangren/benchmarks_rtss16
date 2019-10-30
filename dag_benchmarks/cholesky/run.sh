#!/bin/bash
ARGS="n 2000"
#ARGS="n 3000"
#ARGS="n 4000"
#ARGS="n 5000"
#ARGS="n 6000"

./ch_cilk 1 ${ARGS}
#./ch_omp 1 ${ARGS}

./ch_cilk 32 ${ARGS}
#./ch_omp 32 ${ARGS}

./ch_cilk 16 ${ARGS}
#./ch_omp 16 ${ARGS}

./ch_cilk 8 ${ARGS}
#./ch_omp 8 ${ARGS}

./ch_cilk 4 ${ARGS}
#./ch_omp 4 ${ARGS}

./ch_cilk 12 ${ARGS}
#./ch_omp 12 ${ARGS}

./ch_cilk 24 ${ARGS}
#./ch_omp 24 ${ARGS}

./ch_cilk 20 ${ARGS}
#./ch_omp 20 ${ARGS}

./ch_cilk 28 ${ARGS}
#./ch_omp 28 ${ARGS}

all: omp cilk

omp: synch_task_omp.cpp timespec_functions.cpp
	g++ -fopenmp synch_task_omp.cpp timespec_functions.cpp -o task_omp -lrt

cilk: synch_task_cilk.cpp timespec_functions.cpp
	g++ -fcilkplus synch_task_cilk.cpp timespec_functions.cpp -o task_cilk -lrt -lcilkrts

clean: 
	rm task_omp task_cilk
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <omp.h>
#include <limits.h>


#define SEC_PER_NS 1000000000
#define NUM_ITERS 100000

int main(int argc, char* argv[]) {

	if (argc != 3 && argc != 2) {
		printf("Usage: program <#cores> [#runs]\n");
		return -1;
	}

	int num_cores = atoi(argv[1]);

	int num_iters;
	if (argc == 2) {
		num_iters = NUM_ITERS;
	} else if (argc == 3) {
		num_iters = atoi(argv[2]);		
	}

	//	printf("sizeof(unsigned long long) = %d\n", sizeof(unsigned long long));
	
	// Total record time in microseconds
	unsigned long long total_diff = 0;
	unsigned long long max = 0;
	unsigned long long min = ULLONG_MAX;
	struct timespec start, end;

	
	// Set the number of OMP threads
	omp_set_num_threads(num_cores);

	
	for (int k=0; k<num_iters; k++) 
		{
			/*
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
			for (int i=0; i<num_cores; i++);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

			unsigned long long diff_serial = ((end.tv_sec*SEC_PER_NS + end.tv_nsec) - (start.tv_sec*SEC_PER_NS + start.tv_nsec));
			*/
			unsigned long long diff_serial = 0;
			
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
#pragma omp parallel for
			for (int i=0; i<num_cores; i++) {
				//int num_threads = omp_get_num_threads();
				//printf("Number of threads: %d\n", num_threads);
			}
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

			unsigned long long diff_omp = ((end.tv_sec*SEC_PER_NS + end.tv_nsec) - (start.tv_sec*SEC_PER_NS + start.tv_nsec));

			if (diff_omp < diff_serial) {
				printf("Overhead is negative !!!\n");
				printf("Serial time: %llu\n", diff_serial);
				printf("Parallel time: %llu\n", diff_omp);
				return -1;
			}

			// Overhead for a single run in microseconds
			unsigned long long diff = (diff_omp - diff_serial)/1000;

			total_diff += diff;

			if (max < diff)
				max = diff;
			if (min > diff)
				min = diff;
		}

	unsigned long long average = total_diff/num_iters;
	
	printf("Average Time: %llu microseconds\n", average);
	printf("Max Time: %llu microseconds\n", max);
	printf("Min Time: %llu microseconds\n", min);

	return 0;
}

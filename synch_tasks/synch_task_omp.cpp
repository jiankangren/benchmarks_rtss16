#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "timespec_functions.h"


using namespace std;

#define USEC_IN_SEC 1000000
#define NSEC_IN_SEC 1000000000

// Maximum 32 cores on Shakespeare
#define MAX_NUM_CORES 32

// Run the measurement 1000 times
// Ignore the first 10 runs for warming up (caches, ...)
#define NUM_RUNS 1000
#define WARM_UP_RUNS 10

struct Segment {
	int num_strands;
	int seg_len; // Length in nanoseconds
};

timespec usec_to_timespec(int len_in_usec) {
	timespec t;

	if (len_in_usec < USEC_IN_SEC) {
		t.tv_sec = 0;
		t.tv_nsec = len_in_usec * 1000;
	} else {
		t.tv_sec = len_in_usec / USEC_IN_SEC;
		t.tv_nsec = (len_in_usec % USEC_IN_SEC) * 1000;
	}

	return t;
}

timespec nsec_to_timespec(int len_in_nsec) {
	timespec t;

	if (len_in_nsec < NSEC_IN_SEC) {
		t.tv_sec = 0;
		t.tv_nsec = len_in_nsec;
	} else {
		t.tv_sec = len_in_nsec / NSEC_IN_SEC;
		t.tv_nsec = len_in_nsec % NSEC_IN_SEC;
	}

	return t;
}


/* This program takes the following parameters: 
 *  - an input file of the synchronous task
 *  - the number of processors it runs on
 *  - the number of sample runs (optional, default is 1000 times)
 */
int main(int argc, char* argv[]) {

	if (argc != 4 && argc != 5) {
		cout << "Usage: program <path_to_input_file> <output_folder> <number_of_cores> [number_of_runs]" << endl;
		return -1;
	}

	// Read number of times to run the synchronous structure
	int num_runs = NUM_RUNS + WARM_UP_RUNS;
	if (argc == 5) {
		num_runs = atoi(argv[4]) + WARM_UP_RUNS;
		if (num_runs <= WARM_UP_RUNS) {
			num_runs = NUM_RUNS + WARM_UP_RUNS;
			cout << "The number of runs must be positive! Use default value (1000 times) !" << endl;
		}
	}

	// Read number of threads (cores) to run it
	int num_cores = atoi(argv[3]);
	if (num_cores <= 0 || num_cores > MAX_NUM_CORES) {
	  cout << "The number of cores must be from 1 to " << MAX_NUM_CORES << " !" << endl;
		return -1;
	}

	// Read the file that contains the synchronous structure
	ifstream ifs(argv[1]);
	if (!ifs.is_open()) {
		cout << "Open file " << argv[1] << " failed !" << endl;
		return -1;
	}

	string line;
	if (!getline(ifs, line)) {
		cout << "Get line failed !" << endl;
		return -1;
	}

	istringstream iss(line);
	int num_segments;
	iss >> num_segments;


	struct Segment segments[num_segments];
	int num_strands;
	int seg_len;
	for (int i=0; i<num_segments; i++) {
		iss >> num_strands;
		iss >> seg_len;

		segments[i].num_strands = num_strands;
		segments[i].seg_len = seg_len;
	}


	/*
	// Set the task's affinity and OpenMP environment
	// Bind the task to its desired number of cores, start from core 0
	cpu_set_t mask;
	CPU_ZERO(&mask);
	for (unsigned i=0; i<num_cores; i++) {
		CPU_SET(i, &mask);
	}

	int ret = sched_setaffinity(getpid(), sizeof(mask), &mask);
	if (ret != 0) {
		cout << "Set affinity failed !" << endl;
		return -1;
	}
	*/

	/*
	sched_param sp;
	sp.sched_priority = 98;
	ret = sched_setscheduler(getpid(), SCHED_FIFO, &sp);
	if (ret != 0) {
		cout << "Set SCHED_FIFO scheduler failed !" << endl;
		return -1;
	}
	*/

	// Set the number of threads in the team to the number of cores
	omp_set_dynamic(0);
	omp_set_num_threads(num_cores);
	
	// Set OpenMP schedule
	// Note: this function only applies when the scheduler of the 
	// for loop is set to runtime
	//	omp_set_schedule(omp_sched_dynamic, 1);

	// Write data to a file
	string out_folder = argv[2];
	stringstream ss;
	ss << out_folder << "/omp_" << num_cores << "cores.dat";
	ofstream ofs(ss.str().c_str());

	// Start executing synchronous task
	timespec start, end;

	for (int j=0; j<num_runs; j++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		//	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		for (int i=0; i<num_segments; i++) {
			num_strands = segments[i].num_strands;
			seg_len = segments[i].seg_len;
			
			timespec len = nsec_to_timespec(seg_len);
			
#pragma omp parallel for schedule(dynamic, 1)
			for (int j=0; j<num_strands; j++) {
				busy_work(len);
			}
		}
		//	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
		clock_gettime(CLOCK_MONOTONIC, &end);
		
		timespec diff;
		ts_diff(start, end, diff);
		
		//		cout << "Execution time: " << diff.tv_sec << " seconds : " <<  diff.tv_nsec/1000 << " microseconds" << endl;
		//		ofs << diff.tv_sec << " " << diff.tv_nsec/1000 << endl;

		if (j >= WARM_UP_RUNS) {
			ofs << (diff.tv_sec*USEC_IN_SEC + diff.tv_nsec/1000) << endl;
		}
	}

	ofs.close();

	return 0;
}

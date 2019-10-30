#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cilk.h>
#include <cilk_api.h>
#include "timespec_functions.h"


using namespace std;

#define USEC_IN_SEC 1000000
#define NSEC_IN_SEC 1000000000

// Maximum 32 cores on Shakespeare
#define MAX_NUM_CORES 32

// Default number of runs is 1000
#define NUM_RUNS 1000
#define WARM_UP_RUNS 10

struct Segment {
	int num_strands;
	int seg_len;
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



int main(int argc, char* argv[]) {

	if (argc != 4 && argc != 5) {
		cout << "Usage: program <path_to_input_file> <output_folder> <num_workers> [number_of_runs]" << endl;
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

	int nthreads = atoi(argv[3]);
	if (nthreads <= 0 || nthreads > MAX_NUM_CORES) {
		cout << "Number of Cilk Plus workers must be from 1 to " << MAX_NUM_CORES << " !" << endl;
		return -1;
	}

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
	// Set affinity for the Cilk Plus program
	cpu_set_t mask;
	CPU_ZERO(&mask);
	for (unsigned i=0; i<nthreads; i++) {
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

	// Write data to a file
	string out_folder = argv[2];
	stringstream ss;
	ss << out_folder << "/cilk_" << nthreads << "cores.dat";
	ofstream ofs(ss.str().c_str());

	// Set the number of Cilk Plus workers
	__cilkrts_end_cilk();
	__cilkrts_set_param("nworkers", argv[3]);

	// Start executing synchronous task
	timespec start, end;
	
	for (int j=0; j<num_runs; j++) {
		//	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (int i=0; i<num_segments; i++) {
			num_strands = segments[i].num_strands;
			seg_len = segments[i].seg_len;
			
			timespec len = nsec_to_timespec(seg_len);
			
			//#pragma cilk grainsize = 100
			// Use cilk_for to parallelize executions of strands
			cilk_for (int j=0; j<num_strands; j++) {
				busy_work(len);
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		//	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
		
		// Check if the number of workers is still the same
		//	cout << "Number of workers: " << __cilkrts_get_nworkers() << endl;
		
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

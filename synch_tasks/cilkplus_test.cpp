/*
 * This is a simple program to learn how to set the number of workers for 
 * Cilk Plus runtime. 
 * Discussed at: https://software.intel.com/en-us/forums/intel-cilk-plus/topic/265680
 */

#include <iostream>
#include <stdlib.h>
#include <cilk.h>
#include <cilk_api.h>

using namespace std;

int main(int argc, char* argv[]) {
	
	if (argc != 2) {
		cout << "Usage: program <num_threads>" << endl;
		return -1;
	}

	int nthreads = atoi(argv[1]);
	if (nthreads <= 0 || nthreads > 48) {
		cout << "The number of threads must be from 1 to 48 !" << endl;
		return -1;
	}

	cout << "Default number of workers: " << __cilkrts_get_nworkers() << endl;
	
	__cilkrts_end_cilk();
	__cilkrts_set_param("nworkers", argv[1]);

	cout << "Current number of workers: " << __cilkrts_get_nworkers() << endl;

	cilk_for (int i=0; i<100; i++) {
		if ((i+1) % 20 == 0) {
			cout << "Number of workers: " << __cilkrts_get_nworkers() << endl;
		}
	}
	

	return 0;
}

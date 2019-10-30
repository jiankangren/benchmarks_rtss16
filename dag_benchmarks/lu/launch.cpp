/*
 * This program sets up the shared memory region 
 * for recording execution time
 */
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <assert.h>
#include <sys/wait.h>
#include "common.h"


int init_shm_mem() {
	int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		fprintf(stderr, "Open shared mem failed: %s\n", strerror(errno));
		return -1;
	}

	int size = sizeof(struct ExeTimeData);
	
	int ret = ftruncate(fd, size);
	if (ret == -1) {
		fprintf(stderr, "Truncate shared mem failed: %s\n", strerror(errno));
		return -1;
	}

	struct ExeTimeData *data = (struct ExeTimeData*) mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "Map shared memory failed: %s\n", strerror(errno));
		return -1;
	}

	// Init shared mem
	memset(data, 0, size);
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&data->mutex, &mutex_attr);
	data->count = 0;
	
	// Close shared mem
	close(fd);
	ret = munmap(data, size);
	if (ret == -1) {
		fprintf(stderr, "Unmap shared memory failed: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}



/*
 * Example: ./proc_launch 2 ./ch_cilk 2 n 3000
 * Note that the number indicated after ./proc_launch must be equal the number 
 * indicated after ./ch_cilk (in this example, it is 2)
 */
int main(int argc, char *argv[]) {
	
	if (argc < 3) {
		printf("Usage: %s <num_threads> <command line for the forked programs>\n", argv[0]);
		return -1;
	}

	// Init shared memory to record data
	int ret = init_shm_mem();
	if (ret < 0)
		return ret;

	int num_threads = atoi(argv[1]);

	// Number of processes that can run in parallel
	// Each process requires num_threads cores
	int num_par_proc = TOTAL_NUM_CORES / num_threads;

	int remaining = TOTAL_NUM_ITERS % num_par_proc;
	
	// Number of times to run processes in parallel
	int num_outer_loops;
	
	// Number of processes in the last run
	int num_par_proc_last;
	
	if (remaining == 0) {
		num_outer_loops = TOTAL_NUM_ITERS / num_par_proc;
		num_par_proc_last = num_par_proc;
	} else {
		num_outer_loops = TOTAL_NUM_ITERS / num_par_proc + 1;
		num_par_proc_last = remaining;
	}
	
	
	std::vector<const char*> args_vector;
	for (int i=2; i<argc; i++) {
		args_vector.push_back(argv[i]);
	}
	args_vector.push_back(NULL);

	// Core range allowed to run on
	const int low_core = 2;
	const int high_core = 31;
	
	for (int i=0; i<num_outer_loops; i++) {
		//		printf("%d - Starting new parallel run...\n", i);
		int bound;
		if (i < num_outer_loops-1) {
			bound = num_par_proc;
		} else {
			// The last run may have different number of iterations
			bound = num_par_proc_last;
		}

		int next_core = low_core;
		for (int j=0; j<bound; j++) {
			int end_core = next_core + num_threads - 1;
			assert(end_core <= high_core);

			//			printf("Cores: %d - %d\n", next_core, end_core);
		
			pid_t pid = fork();
			if (pid < 0) {
				printf("Failed to fork: %s\n", strerror(errno));
				kill(0, SIGTERM);
				return -1;
			}

			if (pid == 0) {
				// Add "taskset -c <core range>" to the command line
				std::stringstream ss;
				ss << next_core << "-" << end_core;
				args_vector.insert(args_vector.begin(), ss.str().c_str());
				args_vector.insert(args_vector.begin(), "-c");
				args_vector.insert(args_vector.begin(), "/bin/taskset");
				
				/*
				std::stringstream tmp;
				for (int k=0; k<args_vector.size(); k++) {
					tmp << args_vector[k] << " ";
				}
				
				std::cout << tmp.str() << std::endl;
				*/

				execv("/bin/taskset", const_cast<char **> (&args_vector[0]));

				// Error if execv returns
                perror("Execv-ing a new task failed");
                kill(0, SIGTERM);
				return -1;
			}

			// In parent process
			next_core = end_core + 1;

		} // Inner for-loop

		printf("Waiting for forked processes to finish ...\n");
		// Wait for all children processes to finish before running the next parallel run
		while (!(wait(NULL) == -1 && errno == ECHILD));

	} // Outer for-loop

	
	// Destroy shared mem
	//	destroy_shm_mem();

	return 0;
}

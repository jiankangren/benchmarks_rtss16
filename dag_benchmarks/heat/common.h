#include <stdint.h>
#include <pthread.h>


// Total number of iterations for each data input
// Note that there are multiple processes running for 
// each input, and each process only runs 1 iteration.
// They will write the result to a shared memory.
// The last process that runs the last iteration will 
// write the shared memory to a file and destroy the shared mem.
#define TOTAL_NUM_ITERS 100

// Run in core 2 to 31
#define TOTAL_NUM_CORES 30

// Shared mem name
#define SHM_NAME "/RTSS16_RECORD_EXE_TIME"

// The shared structure between processes
// Use pthread mutex to protect the data
struct ExeTimeData {
	pthread_mutex_t mutex; // to protect the data
	int count; // track number of recorded iterations
	uint32_t data[TOTAL_NUM_ITERS]; // each recorded exe time is stored as a 32-bit int
};

// Note that the shared mem is initialized by a separate process
// before the working processes access to it.
// Open the shared mem to record data
struct ExeTimeData* get_shm_mem();

// Unmap the shared memory region
void unmap_shm_mem(void *mem);

// This is called by the process running the last iteration to 
// destroy the shared mem (after writing the data to file)
void destroy_shm_mem();

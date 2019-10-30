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
#include "common.h"

using namespace std;

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



int main(int argc, char *argv[]) {
	
	int ret = init_shm_mem();
	if (ret < 0)
		return ret;
}

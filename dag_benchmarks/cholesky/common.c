#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "common.h"


// Get and map the shared mem region
struct ExeTimeData* get_shm_mem() {
	
	int fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		fprintf(stderr, "Open shared mem failed: %s\n", strerror(errno));
		return NULL;
	}

	int size = sizeof(struct ExeTimeData);
	
	struct ExeTimeData *data = (struct ExeTimeData*) mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "Map shared memory failed: %s\n", strerror(errno));
		return NULL;
	}

	// Don't need to use fd
	close(fd);

	return data;
}


// Unmap shared mem
void unmap_shm_mem(void *mem) {

	int size = sizeof(struct ExeTimeData);
	int ret = munmap(mem, size);
	if (ret == -1) {
		fprintf(stderr, "Unmap shared memory failed: %s\n", strerror(errno));
	}
}

// Destroy shared mem
void destroy_shm_mem() {

	int ret = shm_unlink(SHM_NAME);
	if (ret < 0) {
		fprintf(stderr, "Destroy shared memory failed: %s\n", strerror(errno));
	}
}

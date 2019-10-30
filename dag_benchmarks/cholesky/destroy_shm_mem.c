/*
 * Simple program to check if a shared mem is 
 * already exists. If it is destroy the shared mem.
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
#include "common.h"


int main() {
	
	int fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		fprintf(stderr, "Open shared mem failed: %s\n", strerror(errno));
		return -1;
	}

	if (fd < 0) {
		fprintf(stderr, "Shared mem does not exists: %s\n", strerror(errno));
		return -1;
	}

	fprintf(stdout, "Shared mem exits. Going to destroy it!\n");

	int ret = shm_unlink(SHM_NAME);
	if (ret < 0) {
		fprintf(stderr, "Destroy shared mem failed: %s\n", strerror(errno));
		return -1;
	}
	
	return 0;
}

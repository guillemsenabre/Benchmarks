#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define SIZE 512 // 512 B
#define ALIGNMENT 512 // 512 B
#define FREQ 1848 // Machine dependent CPU frequency [MHz]

#define CLOCK CLOCK_MONOTONIC_RAW

uint64_t get_ns();
double get_cycles(uint64_t latency);

int main() {	
	void *aligned_memory_ptr;
	
	size_t size = SIZE;
	size_t alignment = ALIGNMENT;

	uint64_t start;
	uint64_t end;

	uint64_t latency;
	double cycles;

	ssize_t ret_write;
	int ret_memalign;

	int fd = open("./files/disk_lat", O_WRONLY | O_CREAT | O_SYNC | O_DIRECT, 0666 );
	
	if (fd < 0) { perror("open"); return 1; }
	
	ret_memalign = posix_memalign(&aligned_memory_ptr, alignment, size);

	if (ret_memalign != 0) { perror("memalign"); return 1;}
	
	memset(aligned_memory_ptr, 'a', size);

	start = get_ns();
	ret_write = write(fd, aligned_memory_ptr, size);
	end = get_ns();

	latency = end - start;
	cycles = get_cycles(latency);	
	

	printf("Disk %d write lat: %.2ld ns/access\n", SIZE, latency);
	printf("Disk %d write cycles: %.2f cycles/access\n", SIZE, cycles);

	free(aligned_memory_ptr);

	return 0;
}

uint64_t get_ns() {
	struct timespec ts;
	clock_gettime(CLOCK, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

double get_cycles(uint64_t latency) {
	double freq_ghz = (double)FREQ / 1000;

	return (double)latency * freq_ghz;
}

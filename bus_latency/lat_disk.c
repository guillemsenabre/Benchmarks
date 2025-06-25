#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define SIZE (512) // 4 KiB
#define ALIGNMENT 512 // 512 B
#define FREQ 1848 // HiFive p550 freq [MHz]
#define ITER 100

#define CLOCK CLOCK_MONOTONIC_RAW

uint64_t get_ns();
double get_cycles(uint64_t latency);

int main() {	
	void *aligned_memory_ptr;
	
	size_t size = SIZE;
	size_t alignment = ALIGNMENT;

	uint64_t start;
	uint64_t end;

	uint64_t count = 0;
	uint64_t latency;
	uint64_t diff;
	uint64_t bigger;
	uint64_t smaller;
	double cycles;

	ssize_t ret_write;
	int ret_memalign;

	int fd_1 = open("./files/disk_lat", O_WRONLY | O_CREAT | O_SYNC | O_DIRECT, 0666 );
	FILE *f_ns = fopen("./files/plot_ns", "w" );
	FILE *f_cycles = fopen("./files/plot_cycles", "w");
	
	if (fd_1 < 0) { perror("open"); return 1; }
	if (!f_ns) { perror("fopen"); return 1; }
	if (!f_cycles) { perror("fopen"); return 1; }
	
	for (int i = 0; i < ITER; i++) {
		ret_memalign = posix_memalign(&aligned_memory_ptr, alignment, size);

        	if (ret_memalign != 0) { perror("memalign"); return 1;}

		memset(aligned_memory_ptr, 'a', size);
		
		diff = 0;

		start = get_ns();
		ret_write = write(fd_1, aligned_memory_ptr, size);
		end = get_ns();
		
		if (ret_write != size) { perror("write"); return 1;}
		
		free(aligned_memory_ptr);
		
		diff = (end - start);

		if ( i == 0) {
			bigger = smaller = diff;
		}

		else if ( i > 0) {
			if (diff > bigger) {
				bigger = diff;
			} else if ( diff < smaller) {
				smaller = diff;
			}
		}

		count += (end - start);
		
		fprintf(f_ns, "%ld\n", diff);
		fprintf(f_cycles, "%f\n", get_cycles(diff));
	}

	latency = count / ITER;
	cycles = get_cycles(latency);

	printf("Disk %d avg write lat: %.2ld ns/access\n", SIZE, latency);
	printf("Disk %d avg write cycles: %.2f cycles/access\n", SIZE, cycles);
	printf("Disk %d write biggest lat: %.2ld ns/access\n", SIZE, bigger);
	printf("Disk %d write smallest lat: %.2ld ns/access\n", SIZE, smaller);

	fclose(f_ns);
	fclose(f_cycles);

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

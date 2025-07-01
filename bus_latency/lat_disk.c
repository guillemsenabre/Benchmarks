#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#define SIZE 512 // Default to 512 Bytes
#define ALIGNMENT 512 // 512 B
#define ITER 100

#define CLOCK CLOCK_MONOTONIC_RAW

uint64_t get_ns();
uint64_t analyse_machine_cpu_freq();
double get_cycles(uint64_t latency, uint64_t freq);
int check_args(char **argv, size_t *size); // TODO: Consider passing argc as well in order to improve portability and robustness.

int main(int argc, char *argv[]) {
	size_t size = SIZE;

	if (argc > 1) {
		if (check_args(argv, &size) != 0) {return 1;}
	}

	uint64_t freq = analyse_machine_cpu_freq();

	void *aligned_memory_ptr;
	
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
	
	if (fd_1 < 0) {perror("open"); return 1;}
	if (!f_ns) {perror("fopen"); return 1;}
	if (!f_cycles) {perror("fopen"); return 1;}
	
	for (int i = 0; i < ITER; i++) {
		ret_memalign = posix_memalign(&aligned_memory_ptr, alignment, size);

        	if (ret_memalign != 0) {perror("memalign"); return 1;}

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
		fprintf(f_cycles, "%f\n", get_cycles(diff, freq));
	}

	latency = count / ITER;
	cycles = get_cycles(latency, freq);

	printf("Disk %d avg write lat: %.2ld ns/access\n", SIZE, latency);
	printf("Disk %d avg write cycles: %.2f cycles/access\n", SIZE, cycles);
	printf("Disk %d write biggest lat: %.2ld ns/access\n", SIZE, bigger);
	printf("Disk %d write smallest lat: %.2ld ns/access\n", SIZE, smaller);
	
	//fclose(fd_1); TODO: Ok, but why
	fclose(f_ns);
	fclose(f_cycles);

	return 0;
}

uint64_t get_ns() {
	struct timespec ts;
	clock_gettime(CLOCK, &ts);
	
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

double get_cycles(uint64_t latency, uint64_t freq) {
	double freq_ghz = (double)freq / 1000000;

	return (double)latency * freq_ghz;
}

int check_args(char **argv, size_t *size) {
	if (sscanf(argv[1], "%zu", size) != 1) {
                perror("sscanf");
                return 1;
        }

        if (*size % (size_t)ALIGNMENT != 0) {
                fprintf(stderr, "alignment: size must be a multiple of %d\n", ALIGNMENT);
        	return 1;
	}

	return 0;
}

uint64_t analyse_machine_cpu_freq() {
	struct timespec remaining, request = {0, 100000000};
	int iter = 3;
	uint64_t cur_freq[3];
	uint64_t acc = 0;

	for (int i = 0; i < iter; i++) {
		FILE *f_machine_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
		if (!f_machine_freq) {perror("fopen"); return 1;}

		if (fscanf(f_machine_freq, "%" SCNu64, &cur_freq[i]) != 1) {perror("fscanf"); return 1;}

		if (nanosleep(&request, &remaining) != 0) {perror("nanosleep"); return 1;}

		acc += cur_freq[i];

		fclose(f_machine_freq);
	}

	uint64_t avg = acc/iter;

	if (avg != cur_freq[0]) {
		printf("Machine has scalable frequency, revising scaling driver (using %" PRIu64" Hz in the meantime)...\n", cur_freq[0]);
	} else {
		printf("Machine has an static frequency of %" PRIu64", continuing...\n", cur_freq[0]);
	}

	return cur_freq[0];
}

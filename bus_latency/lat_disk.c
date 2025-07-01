#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#define CLOCK CLOCK_MONOTONIC_RAW

uint64_t get_ns(void);
uint64_t analyse_machine_cpu_freq(void);
double get_cycles(uint64_t latency, uint64_t freq);
void usage(void);

int main(int argc, char *argv[]) {
	int iter = 100;
	int opt;
	size_t size = 512;
	size_t alignment = 512;

	while ((opt = getopt(argc, argv, "s:a:i:")) != -1) {
		switch (opt) {
			case 's':
				if (sscanf(optarg, "%zu", &size) != 1) {perror("sscanf"); return 1;}
				break;
			case 'a':
				if (sscanf(optarg, "%zu", &alignment) != 1) {perror("sscanf"); return 1;}
				break;
			case 'i':
				if (sscanf(optarg, "%d", &iter) != 1) {perror("sscanf"); return 1;}
				break;
			default:
				usage();
		}
	}

	if (size % (size_t)alignment != 0) {fprintf(stderr, "alignment: size must be a multiple of %zu\n", alignment); return 1;}

	uint64_t freq = analyse_machine_cpu_freq();

	void *aligned_memory_ptr;
	
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
	
	for (int i = 0; i < iter; i++) {
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

	latency = count / iter;
	cycles = get_cycles(latency, freq);

	printf("Disk %zu avg write lat: %.2ld ns/access\n", size, latency);
	printf("Disk %zu avg write cycles: %.2f cycles/access\n", size, cycles);
	printf("Disk %zu write biggest lat: %.2ld ns/access\n", size, bigger);
	printf("Disk %zu write smallest lat: %.2ld ns/access\n", size, smaller);
	
	//fclose(fd_1); TODO: Ok, but why
	fclose(f_ns);
	fclose(f_cycles);

	return 0;
}

uint64_t get_ns(void) {
	struct timespec ts;
	clock_gettime(CLOCK, &ts);
	
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

double get_cycles(uint64_t latency, uint64_t freq) {
	double freq_ghz = (double)freq / 1000000;

	return (double)latency * freq_ghz;
}

uint64_t analyse_machine_cpu_freq(void) {
	struct timespec remaining, request = {0, 100000000};
	int iter = 3;
	uint64_t cur_freq[iter];
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

void usage(void) {
    (void)fprintf(stderr,
        "usage: ./lat_disk [-a <alignment>] [-i <iterations>] [-s <size>]\n"
        "  -a <alignment>    Set the memory alignment (in bytes)\n"
        "  -i <iterations>   Number of iterations for the test loop\n"
        "  -s <size>         Specify the block size (in bytes) to write to disk\n");
    exit(1);
}


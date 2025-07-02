#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#define CACHE_LINE 64
#define L1_SIZE (64 * 1024)     // 64KB
#define L2_SIZE (1024 * 1024)   // 1MB

#define CORE_ID 3

uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void pin_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);              // clear all CPUs
    CPU_SET(core_id, &cpuset);     // select the core

    // Set affinity of current process (pid = 0 means "this process")
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity");
        exit(1);
    }
}

int main() {
    pin_to_core(CORE_ID); // always use the same core

    // Allocate buffer for L1 test
    char *buf = aligned_alloc(CACHE_LINE, L1_SIZE);
    memset(buf, 0, L1_SIZE);  // warm-up (load into L1)

    // Measure L1 cache hit
    uint64_t start = get_ns();
    for (size_t i = 0; i < L1_SIZE; i += CACHE_LINE)
        buf[i] += 1;
    uint64_t end = get_ns();
    printf("L1 cache hit latency: %.2f ns/access\n", (end - start) / (L1_SIZE / (double)CACHE_LINE));

    // Allocate large buffer to evict L1 + L2
    char *evict = aligned_alloc(CACHE_LINE, 4 * L2_SIZE);
    memset(evict, 1, 4 * L2_SIZE);  // trash cache

    // Re-access to trigger miss
    start = get_ns();
    for (size_t i = 0; i < L1_SIZE; i += CACHE_LINE)
        buf[i] += 1;
    end = get_ns();
    printf("Cache miss latency: %.2f ns/access\n", (end - start) / (L1_SIZE / (double)CACHE_LINE));

    free(buf);
    free(evict);
    return 0;
}
printf("L1 cache hit latency: %.2f ns/access\n", (end - start) / (L1_SIZE / (double)CACHE_LINE));

#include <stdio.h>
#include <stdlib.>
#include <fcntl.h>

#define SIZE (4*1024) // 4 KiB
#define ALIGNMENT 512 // 512 B

int main() {	
	void *aligned_memory;
	
	size_t size = SIZE;
	size_t alignment = ALIGNMENT;


	int fd = open("./files/disk_lat", O_WRONLY | O_CREAT | O_SYNC | O_DIRECT, 0666 );
	
	if (fd < 0) { perror("open"); return 1; }
	

	if (posix_memalign(void &aligned_memory, alignment, size) != 0) {
		perror("memalign");
		return 1;
	}
}


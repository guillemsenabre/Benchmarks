#include <stdio.h>
#include <stdlib.>
#include <fcntl.h>

#define SIZE (4*1024) // 4 KiB


int main() {
	int fd = open("./files/disk_lat", O_WRONLY | O_CREAT | O_SYNC | O_DIRECT, 0666 );
	
	if (fd < 0) { perror("open"); return 1; }
}


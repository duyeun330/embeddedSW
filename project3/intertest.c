#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
int main(void){
	int fd;
	int retn;
	char buf[2] = {0,};
	// stopwatch를 open한다
	fd = open("/dev/stopwatch", O_RDWR);
	if(fd < 0) {
		perror("/dev/stopwatch error");
		exit(-1);
	}
        else { printf("< Stopwatch Device has been detected > \n"); }
	
	retn = write(fd, buf, 2);
	close(fd);

	return 0;
}

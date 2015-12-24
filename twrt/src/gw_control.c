#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

void fan_control(int cmd){
	int fd;

	fd = open("/dev/fan_rt5350", O_RDWR);
	if (fd < 0) {
		printf("failed to open /dev/fan_rt5350\n");
	}
	else {
		if (cmd > 0){
			ioctl(fd,1);
		}
		else {
			ioctl(fd,0);
		}

		if (close(fd) < 0) {
			printf("failed to close /dev/fan_rt5350\n");
		}
	}
}

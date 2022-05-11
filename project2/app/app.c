#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#define HW2_DEVICE "/dev/dev_driver"

static struct ioctl_info{
     unsigned char t_interval;
     unsigned char t_cnt;
     int t_init;
	unsigned char st_name[32];
};

struct ioctl_info info;

int ft_is_valid(int num){
	int flag = 0;
	int tmp;

	if (!(num > 0 && num < 8000))
		return (0);
	while (num != 0) {
		tmp = num % 10;
		if (tmp) {
			if (tmp > 8)
				return (0);
			if (flag)
				return (0);
			else
				flag = 1;
		}
		num /= 10;
	}
	return (1);
}

int main(int argc, char **argv)
{
	int dev;
	unsigned char interval;
	unsigned char count;
	int init;
	if (argc != 4) {
		printf("please input the parameter! \n");
		printf("ex)./app TIMER_INTERVAL TIMER_CNT TIMER_INIT\n");
		return -1;
	}

	interval = atoi(argv[1]);
	if (!(interval > 0 && interval <= 100))
	{
		printf("TIMER INTERVAL INPUT : [1 ~ 100]\n");
		exit(1);
	}
	count = atoi(argv[2]);
	if (!(count > 0 && count <= 100))
	{
		printf("TIMER CNT INPUT : [1 ~ 100]\n");
		exit(1);
	}
	init = atoi(argv[3]);
	if (!ft_is_valid(init)){
		printf("TIMER_INIT INPUT : [0001 ~ 8000]\n");
		exit(1);
	}
	dev = open(HW2_DEVICE, O_RDWR);
	if (dev < 0) {
        printf("Device open error : %s\n", HW2_DEVICE);
        exit(1);
	}
	info.t_interval = interval;
	info.t_cnt = count;
	info.t_init = init;
	strncat(info.st_name, "20161664        Hwang DooYeun   ", 32);
	printf("%d %d %d\n", info.t_interval, info.t_cnt, info.t_init);
	ioctl(dev, 0, &info);
	ioctl(dev, 1);
   	return(0);
}

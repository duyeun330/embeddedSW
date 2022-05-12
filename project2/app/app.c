#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#define SET_OPTION 0
#define COMMAND 1
#define HW2_DEVICE "/dev/dev_driver"

// ioctl을 통해 보내게 될 정보들을 structure로 생성
static struct ioctl_info{
     unsigned char t_interval;
     unsigned char t_cnt;
     int t_init;
	unsigned char st_name[32];
};

struct ioctl_info info;

//init 값이 유효한지 확인하여 유효하지 않으면 0, 유효하면 1을 반환.
int ft_is_valid(int num){
	int flag = 0;
	int tmp;

	if (!(num > 0 && num <= 8000))
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
	
	// 인자가 4가 아니면 오류
	if (argc != 4) {
		printf("please input the parameter! \n");
		printf("ex)./app TIMER_INTERVAL TIMER_CNT TIMER_INIT\n");
		return -1;
	}
	
	// interval의 값이 1~100 사이인지 확인
	interval = atoi(argv[1]);
	if (!(interval > 0 && interval <= 100))
	{
		printf("TIMER INTERVAL INPUT : [1 ~ 100]\n");
		exit(1);
	}

	// count의 값이 1~100 사이인지 확인
	count = atoi(argv[2]);
	if (!(count > 0 && count <= 100))
	{
		printf("TIMER CNT INPUT : [1 ~ 100]\n");
		exit(1);
	}

	// init의 값이 1000~8000사이인지 확인
	init = atoi(argv[3]);
	if (!ft_is_valid(init)){
		printf("TIMER_INIT INPUT : [0001 ~ 8000]\n");
		exit(1);
	}

	//device open error 확인
	dev = open(HW2_DEVICE, O_RDWR);
	if (dev < 0) {
        printf("Device open error : %s\n", HW2_DEVICE);
        exit(1);
	}

	//각 정보들을 info structure에 저장 후 ioctl 호출
	info.t_interval = interval;
	info.t_cnt = count;
	info.t_init = init;
	strncat(info.st_name, "20161664        Hwang DooYeun   ", 32);
	printf("%d %d %d\n", info.t_interval, info.t_cnt, info.t_init);
	ioctl(dev, SET_OPTION, &info);
	ioctl(dev, COMMAND);
   	return(0);
}

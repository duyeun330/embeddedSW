#include "input.h"
#include "hw.h"

void init()
{
	if ((fd_ReadKey = open("/dev/input/event0", O_RDONLY)) == -1)
	{
		printf("Device is not valid\n");
		exit(-1);
	}
	printf("%d %d\n", loc_time.tm_hour, loc_time.tm_min);
	semop(semid, &p1, 1);
	input_buffer->mode = CLOCK;
	input_buffer->switch_num = 0;
	semop(semid, &v2, 1);
}

void input_process()
{
	while (1)
	{
		switch(mode)
		{
			case CLOCK :
				break ;

			default :
				break ;
		}
		break ;
	}	
}

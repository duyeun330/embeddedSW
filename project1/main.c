#include "hw.h"

int set_semaphore() {
	union semun	x;
	int			id;

	x.val = 1;
	id = -1;
	if ((id = semget(SEM_KEY, 2, 0600|IPC_CREAT)) == -1)
		exit(-1);
	if (semctl(id, 0, SETVAL, x) == -1)
		exit(-1);
	x.val = 0;
	if (semctl(id, 1, SETVAL, x) == -1)
		exit(-1);
	return (id);
}

void set_shared_memory() {
	if ((shm_id1 = shmget (SHARED_KEY1, sizeof(struct databuf1), 0600 | IPC_CREAT)) == -1){
		perror("error shmget\n");
		exit(-1);
	}
	if ((shm_id2 = shmget (SHARED_KEY2, sizeof(struct databuf1), 0600 | IPC_CREAT)) == -1){
		perror("error shmget\n");
		exit(-1);
 	}
  	if ((input_buffer = (struct databuf1 *)shmat(shm_id1, 0, 0)) == ERR){
		perror("error shmget\n");
		exit(-1);
	}
	if ((output_buffer = (struct databuf1 *)shmat(shm_id2, 0, 0)) == ERR){
          perror("error shmget\n");
 		exit(-1);
	}
}

void init_device1(){
	int	i;

	// initialize
	for (i = 0; i < 10; i++)
		if (dev_fd[i] > 0)
			close(dev_fd[i]);
	if ((dev_fd[0] = open("/dev/fpga_fnd",O_RDWR)) == -1)
	{
		printf("/dev/fpga_fnd open error\n");
		exit(-1);
	}
	if ((dev_fd[1] = open("/dev/mem",O_RDWR|O_SYNC)) == -1)
	{
		printf("/dev/mem open error\n");
		exit(-1);
	}
	if ((dev_fd[2] = open("/dev/fpga_push_switch",O_RDWR)) == -1)
	{
		printf("/dev/fpga_push_switch open error\n");
		exit(-1);
	}
}

void	init_device2(){}

void	init_device3(){}

void	init_device4(){}

void	init_device_handle(int mode) {
	switch (mode) {
		case CLOCK:
			init_device1();
			break ;

		case COUNTER:
			init_device2();
			break ;

		case TEXT_EDITOR:
			init_device3();
			break ;

		case DRAW_BOARD::
			init_device4();
			break ;

		default :
			break ;
	}
}

void exec_device1(){
	int	hour, min;
	int	i;

	if (!is_edit){
		t = time(NULL);
		loc_time = localtime(&t);
		output_buffer->fnd_buf[0] = (unsigned char)(loc_time.tm_hour / 10 + '0');
		output_buffer->fnd_buf[1] = (unsigned char)(loc_time.tm_hour % 10 + '0');
		output_buffer->fnd_buf[2] = (unsigned char)(loc_time.tm_min / 10 + '0');
		output_buffer->fnd_buf[3] = (unsigned char)(loc_time.tm_min % 10 + '0');	
	}
	else if (is_edit == 1){
		
	}
}

void exec_device2(){}

void exec_device3(){}

void exec_device4(){}


void	exec_device_handle(int mode){
	switch (mode) {
		case CLOCK:
			exec_device1();
			break ;

		case COUNTER:
			exec_device2();
			break ;

		case TEXT_EDITOR:
			exec_device3();
			break ;

		case DRAW_BOARD::
			exec_device4();
			break ;

		default :
			break ;
	}
}
void check_mode(){
	int	mode;
	int	init;
	
	semop(semid, &p2, 1);
	mode = input_buffer->mode;
	init = input_buffer->init;
	if (init)
		input_buffer->init--;
	semop(semid, &v1, 1);
	if (init)
		init_device_handle(mode);
	exec_device_handle(mode);
}
void main_process(){
	while (1)
	{
		check_mode();
	}
}

int main(){
	pid_t pid1, pid2, pid3;

	set_semaphore();
	set_shared_memory();
	if ((pid1 = fork()) == -1) {
		printf("fork failed\n");
		exit(-1);
	}
	else if (pid1 == 0) {
		init();
		input_process();
	}
	else {
		if ((pid2 = fork()) == -1) {
			printf("fork failed\n");
			exit(-1);
		}
		else if (pid2 == 0) {
			output_process();
		}
		else {
			if ((pid3 == fork()) == -1) {
				printf("fork failed\n");
				exit(-1);
			}
			else if (pid3 == 0) {
				main_process();
			}
		}
	}
	return (0);
}

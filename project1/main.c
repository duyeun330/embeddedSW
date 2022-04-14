#include "hw.h"
#include "fpga_dot_font.h"
int set_semaphore() {
	semun	x;
	int		id = -1;
	
	x.val = 1;
	if ((id = semget(SEM_KEY, 2, 0600|IPC_CREAT)) == -1)
		exit(-1);
	if (semctl(id, 0, SETVAL, x) == -1)
		exit(-1);
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
  	if ((input_buffer = (struct databuf1 *)shmat(shm_id1, 0, 0)) == (void *)-1){
		perror("error shmget\n");
		exit(-1);
	}
	if ((output_buffer = (struct databuf1 *)shmat(shm_id2, 0, 0)) == (void *)-1){
          perror("error shmget\n");
 		exit(-1);
	}
}

void open_device(){
	// open readkey
	if ((fd[0] = open(READKEY_ADDRESS, O_RDONLY | O_NONBLOCK)) == -1) {
		printf("%s open error\n", READKEY_ADDRESS);
		exit(-1);
	}
	if ((fd[1] = open(FND_ADDRESS, O_RDWR)) == -1) {
		printf("%s open error\n", FND_ADDRESS);
		exit(-1);
	}
	if ((fd[2] = open(SWITCH_ADDRESS, O_RDWR)) == -1) {
		printf("%s open error\n", SWITCH_ADDRESS);
		exit(-1);
	}
	if ((fd[3] = open(DOT_ADDRESS, O_WRONLY)) == -1) {
		printf("%s open error\n", DOT_ADDRESS);
		exit(-1);
	}
	if ((fd[4] = open(LCD_ADDRESS, O_WRONLY)) == -1) {
		printf("%s open error\n", LCD_ADDRESS);
		exit(-1);
	}
	if ((fd[5] = open(LED_ADDRESS, O_RDWR | O_SYNC)) == -1) {
		printf("%s open error\n", LED_ADDRESS);
		exit(-1);
	}
}

void	close_device(){

}

void	get_current_time(){
	int	hr, min;

	t = time(NULL);
	loc_time = localtime(&t);
	time_buf[0] = loc_time->tm_hour / 10;
	time_buf[1] = loc_time->tm_hour % 10;
	time_buf[2] = loc_time->tm_min / 10;
	time_buf[3] = loc_time->tm_min % 10;
}

void	update_output_time(){
	int	i;
	
	for (i = 0; i < 4; i++)
		output_buffer->fnd_buf[i] = time_buf[i];
}

void	initialize(){

	open_device();
	semop(semid, &p2, 1);
	output_buffer->init = 1;
	output_buffer->mode = 1;
	memset(output_buffer->lcd_buf, 0, sizeof(output_buffer->lcd_buf));
	get_current_time();
	update_output_time();
	output_buffer->led_data = 128;
	semop(semid, &v2, 1);
}

void	input_process(){
	int	rk_buf_size;;
	while (1) {
		usleep(400000);
		semop(semid, &p1, 1);
		rk_buf_size = sizeof(struct input_event);
		read(fd[0], input_buffer->readkey, rk_buf_size * 64);
		read(fd[2], &input_buffer->sw_buf, sizeof(input_buffer->sw_buf));
		semop(semid, &v1, 1);
	}
}

int	main_check_mode(){
	int	is_pressed;
	int	code_num;

	is_pressed = input_buffer->readkey[0].value;
	code_num = input_buffer->readkey[0].code;
	if (is_pressed) {
		switch(code_num) {
			case KEY_BACK :
				// 종료
				exit(1);
				break ;

			case KEY_VOLUME_UP :
				mode = (mode + 2) % 4 + 1;
				main_init = 1;
				break ;

			case KEY_VOLUME_DOWN :
				mode = (mode + 5) % 4;
				main_init = 1;
				break ;

			default :
				break ;
		}
		// readkey memset()
		memset(input_buffer->readkey, 0, sizeof(struct input_event) * 64);
		return (1);
	}
	else {
		memcpy(sw_buf, input_buffer->sw_buf, sizeof(sw_buf));
		return (2);
	}

}

void	mode1_calculate(int	is_init){
	if (is_init) {
		main_init = 1;
		memset(lcd_buf, 0, sizeof(lcd_buf));
		memcpy(dot_buf, fpga_set_blank, sizeof(fpga_set_blank));
		led_data = 128;
	}
	else {
	}
}

void	mode2_calculate(){

}

void	mode3_calculate(){

}

void	mode4_calculate(){

}

void	main_calculate(int mode_check){
	switch (mode) {
		case CLOCK :
			mode1_calculate(mode_check);
			break ;

		case COUNTER :
			mode2_calculate(mode_check);
			break ;

		case TEXT_EDITER :
			mode3_calculate(mode_check);
			break ;

		case DRAW_BOARD :
			mode4_calculate(mode_check);
			break ;
		
		default :
			break ;
	}
}

void	main_process(){
	int	mode_check;
	while (1) {
		// input buffer
		
		semop(semid, &p1, 1);
		mode_check = main_check_mode();
		semop(semid, &v1, 1);

		// calculate
		main_calculate(mode_check);

		// output buffer

		semop(semid, &p2, 1);
		semop(semid, &v2, 1);
	}
}

void	output_mode1(int is_init){
	int checkerr;
	unsigned long	*fpga_addr;
	unsigned char	*led_addr;

	if (is_init){
		if ((checkerr = write(fd[3], &fpga_set_blank, sizeof(fpga_set_blank))) < 0) {
			printf("%s output write error.\n", DOT_ADDRESS);
			exit(-1);
		}
		if ((checkerr = write(fd[4], &output_buffer->lcd_buf, 32)) < 0) {
			printf("%s output write error.\n", LCD_ADDRESS);
			exit(-1);
		}
		output_buffer->init = 0;
	}
	if ((checkerr = write(fd[1], &output_buffer->fnd_buf, 4)) < 0) {
		printf("%s output write error.\n", FND_ADDRESS);
		exit(-1);
	}
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd[5], FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED) {
		printf("%s mmap error\n", LED_ADDRESS);
		exit(-1);
	}
	led_addr = (unsigned char*)((void *)fpga_addr+LED_ADDR);
	*led_addr = output_buffer->led_data;
	munmap(led_addr, 4096);
}

void	output_mode2(int is_init){}
void	output_mode3(int is_init){}
void	output_mode4(int is_init){}
void	output_handler(){
	int	is_init;

	is_init = output_buffer->init;
	switch (output_buffer->mode) {
		case CLOCK :
			output_mode1(is_init);
			break ;
		
		case COUNTER :
			output_mode2(is_init);
			break ;

		case TEXT_EDITER :
			output_mode3(is_init);
			break ;

		case DRAW_BOARD :
			output_mode3(is_init);
			break;

		default :
			close_device();
			break ;
	}
}
void	output_process(){
	while (1){
		usleep(400000);
		semop(semid, &p2, 1);
		output_handler(output_buffer->init);
		semop(semid, &v2, 1);
	}
}
int main(){
	pid_t pid1, pid2, pid3;

	semid = set_semaphore();
	set_shared_memory();
	if ((pid1 = fork()) == -1) {
		printf("fork failed\n");
		exit(-1);
	}
	else if (pid1 == 0) {
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
				initialize();
				main_process();
			}
		}
	}
	return (0);
}

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
  	if ((input_buffer = (struct databuf1 *)shmat(shm_id1, 0, 0)) == ERR){
		perror("error shmget\n");
		exit(-1);
	}
	if ((output_buffer = (struct databuf1 *)shmat(shm_id2, 0, 0)) == ERR){
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

void	get_current_time(unsigned char *t_buf){
	int	hr, min;
	time_t	t;
	struct tm	loc_tt;

	t = time(NULL);
	localtime_r(&t, &loc_tt);
	t_buf[0] = loc_tt.tm_hour / 10;
	t_buf[1] = loc_tt.tm_hour % 10;
	t_buf[2] = loc_tt.tm_min / 10;
	t_buf[3] = loc_tt.tm_min % 10;
	sw_second = loc_tt.tm_sec;
}


void	update_output_time(unsigned char *t_buf){
	int	i;
	
	for (i = 0; i < 4; i++)
		output_buffer->fnd_buf[i] = t_buf[i];
}

void	initialize(){

	open_device();
	semop(semid, &p2, 1);
	output_buffer->init = 1;
	output_buffer->mode = 1;
	memset(output_buffer->lcd_buf, 0, sizeof(output_buffer->lcd_buf));
	get_current_time(time_buf);
	update_output_time(time_buf);
	output_buffer->led_data = 128;
	semop(semid, &v2, 1);
}

void	input_process(){
	int	rk_buf_size;;
	int	get_rk;
	int 	i, flag = 0;
	unsigned char	tmp_sw[9];
	struct input_event	tmp_ev[64];

	rk_buf_size = sizeof(struct input_event);
	while (1) {
		usleep(400000);
		semop(semid, &p1, 1);
		if ((get_rk = read(fd[0], tmp_ev, rk_buf_size * 64)) >= rk_buf_size) {
			memcpy(input_buffer->readkey, tmp_ev, rk_buf_size * 64);
			input_buffer->md[0] = 1;
			printf("hello hello %d %d\n", md[0], md[1]);
		}
		else {
			read(fd[2], &tmp_sw, sizeof(tmp_sw));
			for (i = 0; i < 9; i++) {
				if (tmp_sw[i] == 1) {
					memcpy(input_buffer->sw_buf, tmp_sw, sizeof(tmp_sw));
					input_buffer->md[1] = 1;
					flag = 1;
					printf("hi hi\n");
					break ;
				}
			}
			if (flag == 0) {
				input_buffer->md[0] = input_buffer->md[1] = 0;
			}
			flag = 0;
		}
		semop(semid, &v1, 1);
	}
}

int	main_check_mode(){
	int	is_pressed;
	int	code_num;
	
	memcpy(md, input_buffer->md, sizeof(md));
	if (md[0] == 1)
		is_pressed = READKEY_PRESSED;
	else if (md[1] == 1)
		is_pressed = SWITCH_PRESSED;
	else
		is_pressed = NOTHING_PRESSED;
	
	switch(is_pressed) {
		case READKEY_PRESSED :
			memcpy(readkey, input_buffer->readkey, sizeof(struct input_event) * 64);
			//exit(1);
			break ;

		case SWITCH_PRESSED :
			memcpy(sw_buf, input_buffer->sw_buf, sizeof(sw_buf));
			break ;

		case NOTHING_PRESSED :
			//mode = (mode + 5) % 4;
			break ;

		default :
			break ;
	}
	memset(md, 0, sizeof(md));
	return (is_pressed);
}

void	modify_time(unsigned char *t_buf, int wh){
	unsigned char tmp, tmp2, hr;
	if (wh == 1) {
		tmp = t_buf[0] * 10 + t_buf[1];
		tmp = (tmp + 1) % 24;
		t_buf[0] = tmp / 10;
		t_buf[1] = tmp % 10;
		update_output_time(t_buf);
	}
	else {
		tmp = t_buf[2] * 10 + t_buf[3] + 1;
		if ((hr = tmp / 60) == 1) {	
			tmp2 = t_buf[0] * 10 + t_buf[1];
			tmp2 = (tmp + 1) % 24;
			t_buf[0] = tmp / 10;
			t_buf[1] = tmp % 10;
		}
		tmp %= 60;
		t_buf[2] = tmp / 10;
		t_buf[3] = tmp % 10;
		update_output_time(t_buf);
	}
}

void	mode1_calculate(){
	if (sw_mode == 0) {
		if (sw_buf[0] == 1) {
			printf("sw_mode0    sw_mode0\n");
			sw_mode = 1;
			output_buffer->led_data = 32;
			get_current_time(time_buf1);
		}
	}
	else if (sw_mode == 1) {
		if (sw_buf[2] == 1) {
			modify_time(time_buf, 1);		
		}
		else if (sw_buf[3] == 1) {
			modify_time(time_buf, 0);
		}
		else if (sw_buf[0] == 1) {
			sw_mode = 0;
			output_buffer->led_data = 128;
		}
	}
	if (sw_buf[1] == 1) {
		get_current_time(time_buf);
		update_output_time(time_buf);	
	}

}

int	ret_dec(int wh) {
	if (wh == 1)
		return (10);
	else if (wh == 2)
		return (8);
	else if (wh == 3)
		return (4);
	else 
		return (2);
}

void	add_to_decimal(int m, int wh) {
	int	num = ret_dec(wh);
	switch (m) {
		case 1 :
			n_decimal += num * num;
			break ;
		case 2 :
			n_decimal += num;
			break ;
		case 3 :
			n_decimal += 1;
			break ;
		default :
			break ;
	}
}

void	mode2_calculate(){
	int	i;
	int	tmp;
	int	dec;
	for (i = 1; i <= 3; i++){
		if (sw_buf[i] == 1){
			add_to_decimal(i, sw_mode2);
			break ;
		}
	}
	if (sw_buf[0] == 1) {
		sw_mode2 += 1;
		if (sw_mode2 == 5)
			sw_mode2 = 1;
		if (sw_mode2 ==1)
			output_buffer->led_data = 64;
		else if (sw_mode2 == 2)
			output_buffer->led_data = 32;
		else if (sw_mode2 == 3)
			output_buffer->led_data = 16;
		else
			output_buffer->led_data = 128;
		printf("decimal decimal   %d\n", sw_mode2);
	}
	dec = ret_dec(sw_mode2);
	tmp = n_decimal;
	for (i = 3; i >= 1; i--) {
		output_buffer->fnd_buf[i] = tmp % dec;
		tmp /= dec;
	}
}

void	mode3_calculate(){

}

void	mode4_calculate(){

}


void	mode1_init(){
	output_buffer->init = 1;
	output_buffer->mode = 1;
	memset(output_buffer->lcd_buf, 0, sizeof(output_buffer->lcd_buf));
	update_output_time(time_buf);
	output_buffer->led_data = 128;
}


void	mode2_init(){
	int	i;

	output_buffer->init = 1;
     output_buffer->mode = 2;
     memset(output_buffer->lcd_buf, 0, sizeof(output_buffer->lcd_buf));
	for (i = 0; i < 4; i++)
		output_buffer->fnd_buf[i] = 0;
	output_buffer->led_data = 64;
}


void	mode3_init(){}


void	mode4_init(){}

void	main_init(){
	if (readkey[0].value == KEY_RELEASE)
		return ;
	if (readkey[0].code == KEY_VOLUME_UP) 
	{
		mode = mode + 1;
		if (mode == 5)
			mode = 1;
	}
	else if (readkey[0].code == KEY_VOLUME_DOWN) {
		mode = mode - 1;
		if (mode == 0)
			mode = 4;
	}
	else if (readkey[0].code == KEY_BACK) {
	
	}
	printf("main_init     %d\n", mode);
	switch (mode) {
		case CLOCK :
			mode1_init();
			break ;

		case COUNTER :
			mode2_init();
			break ;

		case TEXT_EDITER :
			mode3_init();
			break ;

		case DRAW_BOARD :
			mode4_init();
			break ;
		
		default :
			break ;
	}

}

void	main_calculate(int mode_check){
	switch (mode) {
		case CLOCK :
			mode1_calculate();
			break ;

		case COUNTER :
			mode2_calculate();
			break ;

		case TEXT_EDITER :
			mode3_calculate();
			break ;

		case DRAW_BOARD :
			mode4_calculate();
			break ;
		
		default :
			break ;
	}

}

void	main_process(){
	int	mode_check;
	int	sec;
	time_t	tmp_t;
	struct tm	tmp_loctime;

	while (1) {
		// input buffer
		usleep(400000);
		semop(semid, &p1, 1);
		mode_check = main_check_mode();
		printf("main  main%d\n", mode_check);
		semop(semid, &v1, 1);

		// calculate
		if (mode_check == 2) {

			// output buffer
	
			semop(semid, &p2, 1);
			main_calculate(mode_check);
			semop(semid, &v2, 1);
		}
		else if (mode_check == 1) {
			semop(semid, &p2, 1);
			main_init();
			semop(semid, &v2, 1);
		}
		else if (mode == 1 && sw_mode == 1) {
			tmp_t = time(NULL);
			localtime_r(&tmp_t, &tmp_loctime);
			if (sw_second != tmp_loctime.tm_sec) {
				get_current_time(time_buf1);
				semop(semid, &p2, 1);
				if (led_data == 32) {
					output_buffer->led_data = led_data = 16;
				}
				else {
					output_buffer->led_data = led_data = 32;
				}
				semop(semid, &v2, 1);
			}
		}

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

void	output_mode2(int is_init){
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
	initialize();
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
				main_process();
			}
		}
	}
	return (0);
}

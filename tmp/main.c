#include "hw.h"

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

void	remove_ipcs() {
	if (shmctl(shm_id1, IPC_RMID, 0) == -1)
		exit(-1);
	if (shmctl(shm_id2, IPC_RMID, 0) == -1)
		exit(-1);
	if (shmctl(semid, IPC_RMID, 0) == -1)
		exit(-1);
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
	int	i;

	for (i = 0; i < 6; i++)
		if (fd[i] > 0) 
			close(fd[i]);
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
	input_buffer->end_flag = output_buffer->end_flag = 0;
	output_buffer->init = 1;
	output_buffer->mode = 1;
	memset(output_buffer->lcd_buf, 0, sizeof(output_buffer->lcd_buf));
	get_current_time(time_buf);
	update_output_time(time_buf);
	output_buffer->led_data = 128;
}

void	input_process(){
	int	rk_buf_size;;
	int	get_rk;
	int 	i, flag = 0;
	unsigned char	tmp_sw[9];
	struct input_event	tmp_ev[64];

	rk_buf_size = sizeof(struct input_event);
	while (!end_flag) {
		usleep(350000);
		semop(semid, &p1, 1);
		if ((get_rk = read(fd[0], tmp_ev, rk_buf_size * 64)) >= rk_buf_size) {
			memcpy(input_buffer->readkey, tmp_ev, rk_buf_size * 64);
			input_buffer->md[0] = 1;
		}
		else {
			read(fd[2], &tmp_sw, sizeof(tmp_sw));
			for (i = 0; i < 9; i++) {
				if (tmp_sw[i] == 1) {
					memcpy(input_buffer->sw_buf, tmp_sw, sizeof(tmp_sw));
					input_buffer->md[1] = 1;
					flag = 1;
					break ;
				}
			}
			if (flag == 0) {
				input_buffer->md[0] = input_buffer->md[1] = 0;
			}
			flag = 0;
		}
		end_flag = input_buffer->end_flag;
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

void	modify_time(unsigned char *t_buf){
	int	sum = 0;

	sum += (int)t_buf[0] * 600;
	sum += (int)t_buf[1] * 60;
	sum += (int)t_buf[2] * 10;
	sum += (int)t_buf[3];
	
	sum += ed_hr * 60 + ed_min;
	sum %= 1440;
	ed_hr = sum / 60;
	ed_min = sum % 60;
	t_buf[3] = (unsigned char)(ed_min % 10);
	t_buf[2] = (unsigned char)(ed_min / 10);
	t_buf[1] = (unsigned char)(ed_hr % 10);
	t_buf[0] = (unsigned char)(ed_hr / 10);
	ed_min = ed_hr = 0;
	update_output_time(t_buf);
}

void	mode1_calculate(){
	if (sw_mode == 0) {
		if (sw_buf[0] == 1) {
			sw_mode = 1;
			output_buffer->led_data = 32;
			get_current_time(time_buf1);
		}
	}
	else if (sw_mode == 1) {
		if (sw_buf[2] == 1) {
			ed_hr += 1;
		}
		else if (sw_buf[3] == 1) {
			ed_min += 1;
		}
		else if (sw_buf[0] == 1) {
			sw_mode = 0;
			modify_time(time_buf);
			output_buffer->led_data = 128;
		}
	}
	if (sw_buf[1] == 1) {
		ed_hr = ed_min = 0;
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
	}
	dec = ret_dec(sw_mode2);
	tmp = n_decimal;
	for (i = 3; i >= 1; i--) {
		output_buffer->fnd_buf[i] = tmp % dec;
		tmp /= dec;
	}
}

int	get_corret_alpha(int num) {
	if (num == 1 && sw_buf[2] == 1) {
		memset(lcd_buf, 0, sizeof(lcd_buf));
		dup_num = idx = 0;
		dup_char = -1;
		is_first = 1;
		count += 2;
		return (1);
	}
	else if (num == 4 && sw_buf[5] == 1) {
		if (is_number) {
			is_number = 0;
			memcpy(dot_buf, fpga_set_A, sizeof(dot_buf));
		}
		else {
			is_number = 1;
			memcpy(dot_buf, fpga_number[1], sizeof(dot_buf));
		}
		dup_num = 0;
		dup_char = -1;
		count += 2;
		return (1);
	}
	else if (num == 7 && sw_buf[8] == 1) {
		idx++;
		count += 2;
		return (1);
	}
	else {
		if (is_number == 0){
			if (num == dup_char) {
				dup_num += 1;
				if (dup_num == 4)
					dup_num = 1;
			}
			else {
				dup_num = 1;
				dup_char = num;
				if (!is_first){
					idx++;
				}
				is_first = 0;
			}
			if (idx >= 32) {
				idx = 31;
				memmove(lcd_buf, lcd_buf + 1, sizeof(lcd_buf) - 1);
			}
			lcd_buf[idx] = alphabet[dup_num][num];
		}
		else {
			if (++idx >= 32) {
				idx = 31;
				memmove(lcd_buf, lcd_buf + 1, sizeof(lcd_buf) - 1);
			}
			if (is_first) {
				idx--;
				is_first = 0;
			}
			lcd_buf[idx] = '0' + num + 1;
		}
		count++;
	}
	return (0);
}

void	mode3_calculate(){
	int	i;
	int	flag = 0;
	int	tmp;

	for (i = 0; i < 9; i++) {
		if (sw_buf[i] == 1) {
			flag = get_corret_alpha(i);
		}
		if (flag)
			break ;
	}
	memcpy(output_buffer->lcd_buf, lcd_buf, sizeof(output_buffer->lcd_buf));
	tmp = count;
	for (i = 3; i >= 0; i--) {
		output_buffer->fnd_buf[i] = tmp % 10;
		tmp /= 10;
	}
	memcpy(output_buffer->dot_buf, dot_buf, sizeof(output_buffer->dot_buf));
}

void	get_dot_matrix(int num){
	unsigned char	tmp = 0;
	int	i;

	if (num == 0) {
		memcpy(dot_buf, fpga_set_blank, sizeof(dot_buf));
		sw_mode4 = 1;
		dx = dy = 0;
	}
	else if (num == 2) {
		if (sw_mode4)
			sw_mode4 = 0;
		else
			sw_mode4 = 1;
	}
	else if (num == 4) {
		dot_buf[dy] ^= (1 << (6 - dx));
	}
	else if (num == 6) {
		memcpy(dot_buf, fpga_set_blank, sizeof(dot_buf));
	}
	else if (num == 8) {
		tmp = (1 << 7) - 1;
		for (i = 0; i < 10; i++)
			dot_buf[i] ^= tmp;
	}
	else {
		if (num == 1)
			dy = (dy + 9) % 10;
		else if (num == 3)
			dx = (dx + 6) % 7;
		else if (num == 5)
			dx = (dx + 8) % 7;
		else
			dy = (dy + 11) % 10;
	}
}

void	mode4_calculate(){
	int	i;
	int	tmp;

	for (i = 0 ; i < 9; i++) {
		if (sw_buf[i] == 1) {
			get_dot_matrix(i);
			break ;
		}
	}
	count4++;
	tmp = count4;
	for (i = 3; i >= 0; i--) {
		output_buffer->fnd_buf[i] = tmp % 10;
		tmp /= 10;
	}
	memcpy(output_buffer->dot_buf, dot_buf, sizeof(output_buffer->dot_buf));
}

void	initialize_device(int wh_mode){
	output_buffer->init = 1;
	output_buffer->mode = wh_mode;
	led_data = output_buffer->led_data = 0;
	memset(lcd_buf, 0, sizeof(lcd_buf));
	memset(output_buffer->lcd_buf, 0, sizeof(output_buffer->lcd_buf));
	memset(dot_buf, 0, sizeof(dot_buf));
	memset(output_buffer->dot_buf, 0, sizeof(output_buffer->dot_buf));
	memset(output_buffer->fnd_buf, 0, sizeof(output_buffer->fnd_buf));
}

void	mode1_init(){
	sw_mode = ed_hr = ed_min = 0;
	initialize_device(CLOCK);	
	update_output_time(time_buf);
	output_buffer->led_data = 128;
}


void	mode2_init(){	
	count = ed_hr = ed_min = 0;
	sw_mode2 = 1;
     initialize_device(COUNTER);
	output_buffer->led_data = 64;
}


void	mode3_init(){	
	dup_num = idx = is_number = 0;
	dup_char = -1;
	is_first = 1;
	count = 0;
	initialize_device(TEXT_EDITER);
	memcpy(dot_buf, fpga_set_A, sizeof(output_buffer->dot_buf));
	memcpy(output_buffer->dot_buf, dot_buf, sizeof(output_buffer->dot_buf));
}

void	mode4_init(){
	get_current_time(time_buf1);
	dx = dy = 0;
     sw_mode4 = 1;
	count4 = 0;
	initialize_device(DRAW_BOARD);
}

void	main_init(){
	if (readkey[0].value == KEY_RELEASE) {
		return ;
	}
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
		end_flag = 1;
		mode = output_buffer->mode = -1;
		output_buffer->end_flag = 1;
	}
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
	time_t	tmp_t;
	struct tm	tmp_loctime;

	while (!end_flag) {
		// input buffer
		usleep(350000);
		semop(semid, &p1, 1);
		mode_check = main_check_mode();
		semop(semid, &v1, 1);

		// calculate
		semop(semid, &p2, 1);
		if (mode_check == 2) {
			main_calculate(mode_check);
		}
		else if (mode_check == 1) {
			main_init();
		}
		else if (mode == 1 && sw_mode == 1) {
			tmp_t = time(NULL);
			localtime_r(&tmp_t, &tmp_loctime);
			if (sw_second != tmp_loctime.tm_sec) {
				get_current_time(time_buf1);	
				if (led_data == 32) {
					output_buffer->led_data = led_data = 16;
				}
				else {
					output_buffer->led_data = led_data = 32;
				}
			}
		}
		else if (mode == 4 && sw_mode4 == 1) {
			tmp_t = time(NULL);
			localtime_r(&tmp_t, &tmp_loctime);
			if (sw_second != tmp_loctime.tm_sec){
				get_current_time(time_buf1);
				output_buffer->dot_buf[dy] ^= (1 << (6 - dx));
			}
		}
		semop(semid, &v2, 1);
	}
	semop(semid, &p1, 1);
	input_buffer->end_flag = 1;
	semop(semid, &v1, 1);
	semop(semid, &p2, 1);
	semop(semid, &v2, 1);
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

void	output_mode3(int is_init){
	int checkerr;
	unsigned long	*fpga_addr;
	unsigned char	*led_addr;

	if (is_init){
		fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd[5], FPGA_BASE_ADDRESS);
		if (fpga_addr == MAP_FAILED) {
			printf("%s mmap error\n", LED_ADDRESS);
			exit(-1);
		}
		led_addr = (unsigned char*)((void *)fpga_addr+LED_ADDR);
		*led_addr = output_buffer->led_data;
		munmap(led_addr, 4096);
		output_buffer->init = 0;
	}
	if ((checkerr = write(fd[1], &output_buffer->fnd_buf, 4)) < 0) {
		printf("%s output write error.\n", FND_ADDRESS);
		exit(-1);
	}
	if ((checkerr = write(fd[3], &output_buffer->dot_buf, sizeof(output_buffer->dot_buf))) < 0) {
		printf("%s output write error.\n", DOT_ADDRESS);
		exit(-1);
	}
	if ((checkerr = write(fd[4], &output_buffer->lcd_buf, 32)) < 0) {
		printf("%s output write error.\n", LCD_ADDRESS);
		exit(-1);
	}
}

void	output_mode4(int is_init){
	int checkerr;
	unsigned long	*fpga_addr;
	unsigned char	*led_addr;

	if (is_init){
		fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd[5], FPGA_BASE_ADDRESS);
		if (fpga_addr == MAP_FAILED) {
			printf("%s mmap error\n", LED_ADDRESS);
			exit(-1);
		}
		led_addr = (unsigned char*)((void *)fpga_addr+LED_ADDR);
		*led_addr = output_buffer->led_data;
		munmap(led_addr, 4096);
		output_buffer->init = 0;
	}
	if ((checkerr = write(fd[1], &output_buffer->fnd_buf, 4)) < 0) {
		printf("%s output write error.\n", FND_ADDRESS);
		exit(-1);
	}
	if ((checkerr = write(fd[3], &output_buffer->dot_buf, sizeof(output_buffer->dot_buf))) < 0) {
		printf("%s output write error.\n", DOT_ADDRESS);
		exit(-1);
	}
	if ((checkerr = write(fd[4], &output_buffer->lcd_buf, 32)) < 0) {
		printf("%s output write error.\n", LCD_ADDRESS);
		exit(-1);
	}
}

void	clean_device(){
	int checkerr;
	unsigned long	*fpga_addr;
	unsigned char	*led_addr;

	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd[5], FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED) {
		printf("%s mmap error\n", LED_ADDRESS);
		exit(-1);
	}
	led_addr = (unsigned char*)((void *)fpga_addr+LED_ADDR);
	*led_addr = 0;
	munmap(led_addr, 4096);
	memset(output_buffer->fnd_buf, 0, sizeof(output_buffer->fnd_buf));
	if ((checkerr = write(fd[1], &output_buffer->fnd_buf, 4)) < 0) {
		printf("%s output write error.\n", FND_ADDRESS);
		exit(-1);
	}
	
	if ((checkerr = write(fd[3], &fpga_set_blank, sizeof(output_buffer->dot_buf))) < 0) {
		printf("%s output write error.\n", DOT_ADDRESS);
		exit(-1);
	}
	memset(output_buffer->lcd_buf, 0, sizeof(output_buffer->lcd_buf));
	if ((checkerr = write(fd[4], &output_buffer->lcd_buf, 32)) < 0) {
		printf("%s output write error.\n", LCD_ADDRESS);
		exit(-1);
	}
}

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
			output_mode4(is_init);
			break;

		default :
			end_flag = output_buffer->end_flag;
			break ;
	}
}

void	output_process(){
	while (!end_flag){
		usleep(350000);
		semop(semid, &p2, 1);
		output_handler();
		semop(semid, &v2, 1);
	}
}

int main(){
	pid_t pid1, pid2, pid3, wpid;
	int	status = 0;

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
			if ((pid3 = fork()) == -1) {
				printf("fork failed\n");
				exit(-1);
			}
			else if (pid3 == 0) {
				main_process();
			}
			else {
				while((wpid = wait(&status)) > 0);
				clean_device();
				close_device();
				remove_ipcs();
			}
		}
	}
	return (0);
}

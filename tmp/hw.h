#ifndef HW_H
#define HW_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <linux/input.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

// DEVICE ADDRESS
#define READKEY_ADDRESS "/dev/input/event0"
#define FND_ADDRESS "/dev/fpga_fnd"
#define SWITCH_ADDRESS "/dev/fpga_push_switch"
#define DOT_ADDRESS "/dev/fpga_dot"
#define LCD_ADDRESS "/dev/fpga_text_lcd"
#define LED_ADDRESS "/dev/mem"
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16

// MODE
#define CLOCK 1
#define COUNTER 2
#define TEXT_EDITER 3
#define DRAW_BOARD 4

#define KEY_VOLUME_UP 115
#define KEY_VOLUME_DOWN 114
#define KEY_BACK 158
#define KEY_RELEASE 0
#define KEY_PRESS 1
#define READKEY_PRESSED 1
#define SWITCH_PRESSED 2
#define NOTHING_PRESSED 0

// SHARED MEMORY & SEMAPHORE KEY-GEN
#define SHARED_KEY1 (key_t) 0x10
#define SHARED_KEY2 (key_t) 0x20
#define SEM_KEY (key_t) 0x30
#define ERR ((void *) -1)

// STRUCT SHARED MEMORY
struct databuf1{
	int	d_size;
	int	init;
	int	mode;
	int	switch_num;
	int	md[2];
	int	end_flag;
	unsigned char	led_data;
	unsigned char	sw_buf[9];
	unsigned char	fnd_buf[4];
	unsigned char	lcd_buf[32];
	unsigned char	dot_buf[10];
	struct input_event readkey[64];
};

struct databuf1 *input_buffer, *output_buffer;

typedef union{
	int	val;
	struct semid_ds *buf;
	ushort *array;
}semun;

// SHARED MEMORY ID & SEMAPHORE ID
int shm_id1, shm_id2, semid;

// SEMAPHORE CONTROL
struct sembuf p1 = {0, -1, SEM_UNDO }, p2 = {1, -1, SEM_UNDO};
struct sembuf v1 = {0, 1, SEM_UNDO }, v2 = {1, 1, SEM_UNDO};

// DEVICE DRIVER FD
int	fd[10];

// For Mode 1
int	sw_mode = 0;
int	ed_hr, ed_min;

// FOR MODE 2
int	n_decimal = 0;
int	sw_mode2 = 1;
int	count = 0;

// FOR MODE 3
int	dup_num = 0;
int	dup_char = -1;
int	idx = 0;
int	is_number = 0;
int	is_first = 1;
char alphabet[4][9] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	{'.', 'A', 'D', 'G', 'J', 'M', 'P', 'T', 'W'},
	{'Q', 'B', 'E', 'H', 'K', 'N', 'R', 'U', 'X'},
	{'Z', 'C', 'F', 'G', 'L', 'O', 'S', 'V', 'Y'}
};

// FOR MODE 4
int	sw_mode4 = 1;
int	count4 = 0;
int	dx, dy;

int	mode = 1;
int	md[2];
int	sw_second;
int	end_flag = 0;
unsigned char	sw_buf[9];
unsigned char	time_buf[4], time_buf1[4];
unsigned char  led_data;
unsigned char  lcd_buf[32];
unsigned char	dot_buf[10];
struct input_event readkey[64];
// Function

int  set_semaphore();
void set_shared_memory();
void remove_ipcs();
void open_device();
void close_device();
void clean_device();

void get_current_time(unsigned char *);
void update_output_time(unsigned char *);

void initialize();


void input_process();

int  main_check_mode();
void modify_time(unsigned char *);
void mode1_calculate();
int  ret_dec(int);
void add_to_decimal(int, int);
void mode2_calculate();
int  get_corret_alpha(int);
void mode3_calculate();
void get_dot_matrix(int);
void mode4_calculate();
void initialize_device(int);
void mode1_init();
void mode2_init();
void mode3_init();
void mode4_init();
void main_init();
void main_calcutate(int);
void main_process();

void output_mode1(int);
void output_mode2(int);
void output_mode3(int);
void output_mode4(int);
void output_handler();
void output_process();

// DOT FONT
unsigned char fpga_number[10][10] = {
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03} // 9
};

unsigned char fpga_set_full[10] = {
	// memset(array,0x7e,sizeof(array));
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f
};

unsigned char fpga_set_blank[10] = {
	// memset(array,0x00,sizeof(array));
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

unsigned char fpga_set_A[10] = {
	// set A
	0x1c, 0x36, 0x63, 0x63, 0x63, 0x7f, 0x7f, 0x63, 0x63, 0x63
};

#endif

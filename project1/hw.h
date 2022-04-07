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
#define Counter 2
#define TEXT_EDITER 3
#define DRAW_BOARD 4

// SHARED MEMORY & SEMAPHORE KEY-GEN
#define SHARED_KEY1 (key_t) 0x10
#define SHARED_KEY2 (key_t) 0x20
#define SEM_KEY (key_t) 0x30
#define ERR ((struct databuf *)-1)

// STRUCT SHARED MEMORY
struct databuf1{
	int	d_size;
	int	init;
	int	mode;
	int	switch_num;
	char	d_buf[1001];
	unsigned char	sw_buf[10];
	unsigned char	fnd_buf[4];
};

struct databuf1 *input_buffer, *output_buffer;

// SHARED MEMORY ID & SEMAPHORE ID
int shm_id1, shm_id2, sem_id;

// SEMAPHORE CONTROL
struct sembuf p1 = {0, -1, SEM_UNDO }, p2 = {1, -1, SEM_UNDO};
struct sembuf v1 = {0, 1, SEM_UNDO }, v2 = {1, 1, SEM_UNDO};

// DEVICE DRIVER FD
int	dev_fd[10];

// For Mode 1
int	is_edit;
struct tm	*loc_time;
time_t	t;
unsigned char	sw_buf[10];

// Function
void set_semaphore();
void set_shared_memory();
void	initialize();

#endif

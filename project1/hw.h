#ifndef HW_H
#define HW_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

// MODE
#define CLOCK 1
#define Counter 2
#define TEXT_EDITER 3
#define DRAW_BOARD 4

// SHARED MEMORY & SEMAPHORE KEY-GEN
#define SHARED_KEY1 (key_t) 0x10
#define SHARED_KEY2 (key_t) 0x20
#define SEM_KEY (key_t) 0x20
#define ERR ((struct databuf *)-1)

// STRUCT SHARED MEMORY
struct databuf1{
	int	d_size;
	int	init;
	int	mode;
	int	switch_num;
	char	d_buf[1001];
};

struct databuf1 *input_buffer, *output_buffer;

// SHARED MEMORY ID & SEMAPHORE ID
int shm_id1, shm_id2, sem_id;

// SEMAPHORE CONTROL
struct sembuf p1 = {0, -1, SEM_UNDO }, p2 = {1, -1, SEM_UNDO};
struct sembuf v1 = {0, 1, SEM_UNDO }, v2 = {1, 1, SEM_UNDO};

#endif

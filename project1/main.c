#include "hw.h"

int set_semaphore() {
	union semun	x;
	int			id;

	x.val = 0;
	id = -1;
	if ((id = semget(SEM_KEY, 2, 0600|IPC_CREAT)) == -1)
		exit(-1);
	if (semctl(id, 0, SETVAL, x) == -1)
		exit(-1);
	if (semctl(id, 0, SETVAL, x) == -1)
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

void check_mode(){
	int	mode;

	semop(semid, &p1, 1);
     semop(semid, &v2, 1);
     
	semop(semid, &p1, 1);
     semop(semid, &v2, 1);
}
void main_process(){
	int flag = 0;
	while (1)
	{
		flag = check_mode();
		if (input_buffer->init)
		{
			continue ;
		}
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

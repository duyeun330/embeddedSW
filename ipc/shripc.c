#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

#define SHARED_KEY1 (key_t) 0x10
#define SHARED_KEY2 (key_t) 0x15
#define SEM_KEY (key_t) 0x20
#define IFLAGS (IPC_CREAT )
#define ERR ((struct databuf *)-1)

#define SIZE 2048

struct sembuf p1 = {0, -1, SEM_UNDO }, p2 = {1, -1, SEM_UNDO};
struct sembuf v1 = {0, 1, SEM_UNDO }, v2 = {1, 1, SEM_UNDO};

struct databuf{
	int d_nread;
	char d_buf[SIZE];
};

static int shm_id1, shm_id2, sem_id;

void getseg(struct databuf **p1, struct databuf **p2) {
	if ((shm_id1 = shmget (SHARED_KEY1, sizeof(struct databuf), 0600 | IFLAGS)) == -1){
		perror("error shmget\n");
		exit(1);
	}
	if ((shm_id2 = shmget (SHARED_KEY2, sizeof(struct databuf), 0600 | IFLAGS)) == -1){
		perror("error shmget\n");
		exit(1);
 	}
   	if ((*p1 = (struct databuf *)shmat(shm_id1, 0, 0)) == ERR){
		perror("error shmget\n");
		exit(1);
	}
	if ((*p2 = (struct databuf *)shmat(shm_id2, 0, 0)) == ERR){
          perror("error shmget\n");
 		exit(1);
	}
}

int getsem(void)
{
	union semun	x;
	int		id = -1;
	
	x.val = 0;
	if ((id = semget(SEM_KEY, 2, 0600|IFLAGS)) == -1)
		exit(1);
	if (semctl(id, 0, SETVAL, x) == -1)
		exit(1);
	if (semctl(id, 0, SETVAL, x) == -1)
		exit(1);
	return (id);
}

void remobj()
{
	if (shmctl (shm_id1, IPC_RMID, 0) == -1)
 		exit(1);
  	if (shmctl (shm_id2, IPC_RMID, 0) == -1)
 		exit(1);
	if (shmctl (sem_id, IPC_RMID, 0) == -1)
 		exit(1);
}
//reader

void reader(int semid, struct databuf *buf1, struct databuf *buf2)
{
	while (1)
	{
		buf1->d_nread = read(0, buf1->d_buf, SIZE);
		semop(semid, &v1, 1);
		semop(semid, &p2, 1);
		if (buf1->d_nread <= 0)
			return ;
		buf2->d_nread = read(0, buf2->d_buf, SIZE);
		semop(semid, &v1, 1);
		semop(semid, &p2, 1);
		if (buf2->d_nread <= 0)
			return ;
	}
}

//writer

void writer(int semid, struct databuf *buf1, struct databuf *buf2)
{
	while (1)
	{
		semop(semid, &p1, 1);
		semop(semid, &v2, 1);
		if (buf1->d_nread <= 0)
			return ;
		write(1, buf1->d_buf, buf1->d_nread);
		semop(semid, &p1, 1);
		semop(semid, &v2, 1);
		if (buf2->d_nread <= 0)
			return ;
		write(1, buf2->d_buf, buf2->d_nread);
	}
}


int main(){
	pid_t pid;
	struct databuf *buf1, *buf2;

	sem_id = getsem();
	getseg(&buf1, &buf2);
	switch(pid = fork()){
		case -1:
			perror("fork");
			break ;

		case 0:
			writer(sem_id, buf1, buf2);
			remobj();
			break ;

		default:
			reader(sem_id, buf1, buf2);
			break ;
	}
}


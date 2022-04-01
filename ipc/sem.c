#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>

int main(){
  int semid;
  
  semid = semget((key_t)12345, 1, 0666|IPC_CREAT);

  return (0);
}

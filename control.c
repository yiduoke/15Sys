#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
            (Linux-specific) */
};

int main(int argc, char *argv[]){
  int KEY = ftok("control.c", 11); // creating a key for the .gitignore file 
  int MEMKEY = ftok("main.c", 22);
  int semDes;
  int memDes;
  int* pointer;
  int fd;
  
  // creating shared memory and semaphore and all that shenanigan 
  if(!strncmp(argv[1], "-c", 2)){

    semDes = semget(KEY, 1, IPC_CREAT | IPC_EXCL | 0777); // getting the descriptor;;
    if (semDes < 0 ){//errno
      printf("failed to create semaphore; %s\n", strerror(errno));
      exit(0);
    }

    memDes = shmget(MEMKEY, sizeof(int), IPC_CREAT | IPC_EXCL | 0777);
    if (memDes < 0 ){//errno
      printf("failed to create shared memory; %s\n", strerror(errno));
      exit(0);
    }

    union semun STRUCTURE;
    STRUCTURE.val = 1;
    if (semctl(semDes, 0, SETVAL, STRUCTURE) < 0){
      printf("failed to set semaphore value; %s\n", strerror(errno));
      exit(0);
    }

    if ( (pointer = (int*)shmat(memDes, 0, 0)) < 0){
      printf("failed to attach shared memory; %s\n", strerror(errno));
      exit(0);
    }

    *pointer = 0;
    if (shmdt(pointer) < 0){
      printf("failed to detach shared memory; %s\n", strerror(errno));
      exit(0); 
    }

    if ( (fd = open("poem.txt", O_CREAT | O_TRUNC | O_EXCL | O_RDWR, 0777)) < 0){
      printf("failed to create file; %s\n", strerror(errno));
      exit(0);
    }
    close(fd);
  }

  // viewing by executing cat on the text file
  else if (!strncmp(argv[1], "-v", 2)){
    if (execlp("cat", "cat", "poem.txt", NULL) < 0){
      printf("poem doesn't exist yet\n");
      exit(0);
    }
  }

  // removing shared memory and semaphore
  else if (!strncmp(argv[1], "-r", 2)){
    semDes = semget(KEY, 1, 0644);
    if (semDes < 0){
      printf("failed to get semaphore descriptor; %s\n", strerror(errno));
      exit(0);
    }

    if (semctl(semDes, 0, IPC_RMID) < 0){//errno
      printf("failed to remove semaphore %d\n", KEY);
      exit(0);
    }
    
    memDes = shmget(MEMKEY, sizeof(int), 0644);
    if (memDes < 0){
      printf("failed to get shared memory descriptor; %s\n", strerror(errno));
      exit(0);
    }

    if (shmctl(memDes, IPC_RMID, 0) < 0){
      printf("failed to remove shared memory %d\n", MEMKEY);
      exit(0);
    }
  }
  return 0;
}
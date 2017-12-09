#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
            (Linux-specific) */
};

int main(int argc, char *argv[] ) {
  int KEY = ftok("makefile", 10); // creating a key for the .gitignore file 
  int MEMKEY = ftok("control.c", 14);
  int sem_des;
  int mem_des;
  int fd;
  int* pointer;
  
  if ( argc == 2 && strcmp(argv[1], "-c" ) == 0 ) {
    //create the semaphore & shared memory    
    sem_des = semget( KEY, 1, IPC_CREAT | IPC_EXCL | 0664 );
    printf("semaphore: %d\n", sem_des);
    if ( sem_des == -1 )
      err( -1, "semget" );
    mem_des = shmget( MEMKEY, sizeof(int), IPC_CREAT | IPC_EXCL | 0664 );
    printf("shared memory: %d\n", mem_des);
    if ( mem_des == -1 )
      err( -1, "shmget" );
    
    //initialize the semaphore
    union semun su;
    su.val = 1;
    e = semctl( sem_des, 0, SETVAL, su );
    if ( e == -1 )
      err( -1, "semctl" );
    
    //initialize the shared memory
    pointer = (int *)shmat( mem_des, 0, 0 );
    if ( *pointer == -1 )
      err( -1, "shmat" );

    *pointer = 0;
    e = shmdt( pointer );
    if ( e == -1 )
      err(-1, "mem_des" );
    
    //create the file
    fd = open("story.txt", O_CREAT | O_TRUNC, 0664);
    if ( fd == -1 )
      err( -1, "open" );
    close(fd);    
  }

  else if ( argc == 2 && strcmp(argv[1], "-r") == 0 ) {
    
    //remove the semaphore
    sem_des = semget( KEY, 1, 0664 );
    if ( sem_des == -1 )
      err( -1, "semget" );
      
    e = semctl( sem_des, 0, IPC_RMID );
    if ( e == -1 )
      err( -1, "semctl" );
    
    //remove the shared memory
    mem_des = shmget( MEMKEY, sizeof(int), 0664 );
    if ( sem_des == -1 )
      err( -1, "shmget" );
    e = shmctl( mem_des, IPC_RMID, 0 );
    if ( e == -1 )
      err( -1, "shmctl" );

  }

  return 0;
}
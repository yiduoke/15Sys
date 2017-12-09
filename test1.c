#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>

int main() {

  int KEY = ftok("makefile", 'a');
  int MEMKEY = ftok("makefile", 'b');
  int mem_des, sem_des;
  int fd;
  int e;
  int* pointer;
  char line[256];

  //get the semaphore
  sem_des = semget( KEY, 1, 0666);
  if ( sem_des == -1 )
    err( -1, "semget" );
  
  printf("Attempting to access story...\n");

  //try to gain access
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_flg = SEM_UNDO;
  sb.sem_op = -1; //request resource
  e = semop( sem_des, &sb, 1);
  if ( e == -1 )
    err( -1, "semop" );

  printf( "Access Granted.\n" );

  //get shared memory
  mem_des = shmget( MEMKEY, sizeof(int), 0664 );
  if ( mem_des == -1 )
    err( -1, "shmget" );
  pointer = shmat( mem_des, 0, 0 );
  if ( *pointer == -1 )
    err( -1, "shmat" );

  fd = open( "story.txt", O_RDWR );
  if ( fd == -1 )
    err( -1 , "open" );
  
  printf( "Reading the last %d bytes of the story:\n", *pointer );
  lseek( fd, -1 * *pointer, SEEK_END );
  read( fd, line, sizeof(line) );
  printf( "%s\n", line );

  printf("\nWhat would you like to add: ");
  fgets( line, sizeof(line), stdin );
  *pointer = strlen( line );
  write( fd, line, *pointer );  
  close(fd);
  shmdt( pointer );
  
  sb.sem_op = 1; //give resource back
  e = semop( sem_des, &sb, 1);
  if ( e == -1 )
    err( -1, "semop" );
  
  printf("Releasing resources.\n");

  return 0;
}
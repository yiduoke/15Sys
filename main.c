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

int main(){
    int KEY = ftok("control.c", 115);
    int MEMKEY = ftok("main.c", 22);
    int semDes;
    int memDes;
    int* pointer;
    char last_line[1000];
    struct sembuf OPERATION;
    int fd;

    printf("checking to see if resources are available...\n");
    
    semDes = semget(KEY, 1, 0777);
    if (semDes < 0 ){
        printf("failed to get to the semaphore; %s\n", strerror(errno));
        return 0;
    }

    OPERATION.sem_num = 0;
    OPERATION.sem_flg = SEM_UNDO;//in case I fk up
    OPERATION.sem_op = -1; //blocking
    if (semop(semDes, &OPERATION, 1) < 0){
        printf("failed to access poem resource");
        return 0;
    }

    memDes = shmget(MEMKEY, sizeof(int), 0644);
    if (memDes < 0){
        printf("failed to access shared memory; %s\n", strerror(errno));
        return 0;
    }

    pointer = shmat(memDes, 0, 0);
    if (pointer < 0){
        printf("failed to assign shared memory to pointer\n");
        return 0;
    }

    fd = open("poem.txt", O_RDWR);
    if (fd < 0){
        printf("failed to open poem\n");
        return 0;
    }

    printf("HARK! They are!\n")

    // reading and printing last line
    lseek(fd, -1 * *pointer, SEEK_END);
    read(fd, last_line, sizeof(last_line));
    printf("%s\n", last_line);

    printf("please enter the next verse: ");
    fgets(last_line, sizeof(last_line), stdin);
    *pointer = strlen(last_line);
    write(fd, last_line, *pointer);
    close(fd);
    shmdt(pointer);

    OPERATION.sem_op = 1;
    if (semop(semDes, &OPERATION, 1) < 0){
        printf("failed to open resource poem up\n");
        return 0;
    }
}
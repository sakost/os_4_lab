#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>


#include "shrmem.h"


int main(int argc, char* argv[]) {
    char *filename = NULL;
    size_t len;

    printf("Enter a filename with tests: ");
    if(getline(&filename, &len, stdin) == -1){
        perror("getline");
    }

    filename[strlen(filename) - 1] = '\0';


    int fd = shm_open(BackingFile, O_RDWR | O_CREAT, AccessPerms);
    int file = open(filename, O_RDONLY);
    if (fd == -1 || file == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    close(file);

    sem_t *semptr = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
    if (semptr == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    int val;


    ftruncate(fd, map_size);  // resize file up to mmap size

    caddr_t memptr = mmap(
            NULL,
            map_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            0
    );


    if(memptr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if(sem_getvalue(semptr, &val) != 0){
        perror("sem_getvalue");
        exit(EXIT_FAILURE);
    }

    memset(memptr, '\0', map_size); // clean shared memory

    while(val++ < 2){ // if semaphore already was created, just fill it up to 2
        sem_post(semptr);
    }

    pid_t pid = fork();

    if(pid == 0){
        munmap(memptr, map_size);
        close(fd);
        sem_close(semptr);
        execl("4_lab_child", "4_lab_child", filename, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
    else if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }


    while (true){
        // get the value of semaphore
        if(sem_getvalue(semptr, &val) != 0){
            perror("sem_getvalue");
            exit(EXIT_FAILURE);
        }
        // if child process seems not to even capture shared memory and semaphore
        if(val == 2){
            continue;
        }
        // wait for semaphore
        if(sem_wait(semptr) != 0) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
        // just end of input
        if (memptr[0] == EOF) {
            break;
        }
        // if child process didn't write something to shared memory(we too fast locked semaphore), just skip iteration
        if(memptr[0] == '\0'){
            if(sem_post(semptr) != 0){
                perror("sem_post");
                exit(EXIT_FAILURE);
            }
            continue;
        }

        char* string = (char*)malloc(strlen(memptr) * sizeof(char));
        // copy data to local storage and clean shared memory
        strcpy(string, memptr);
        memset(memptr, '\0', map_size);
        if(sem_post(semptr) != 0){ // unlock semaphore to let child work
            perror("sem_post");
            exit(EXIT_FAILURE);
        }
        // print the result
        printf("%s\n", string);
        free(string);
    }


    // some error checks...
    int status;
    if(wait(&status) == -1){
        perror("wait");
        exit(EXIT_FAILURE);
    }
    if(!WIFEXITED(status) || (WIFEXITED(status) && (WEXITSTATUS(status)) != 0)){
        fprintf(stderr, "Some error occurred in child process\n");
        return 1;
    }

    // cleanup
    if(munmap(memptr, map_size) != 0){
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    close(fd);
    if(sem_close(semptr) != 0){
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
    if(shm_unlink(BackingFile) != 0){
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}

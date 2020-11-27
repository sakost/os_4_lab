//
// Created by sakost on 23.09.2020.
//
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>
#include <stdbool.h>

#include "shrmem.h"

void send_parent(caddr_t memptr, sem_t* semptr, const char *empty_string, double res){
    while(true) {
        if (sem_wait(semptr) == 0) {
            if(strcmp(memptr, empty_string) != 0) {
                if (sem_post(semptr) != 0) {
                    perror("sem_post");
                    exit(EXIT_FAILURE);
                }
                continue;
            }
            sprintf(memptr, "%lf\n", res);
            if (sem_post(semptr) != 0) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }
            break;
        } else {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
    }
}



double m_eps (void)
{
    double e = 1.0;
    while (1.0 + e / 2.0 > 1.0) e /= 2.0;
    return e;
}


int main(int argc, char **argv){
    const double eps = m_eps();
    double a;
    char c;
    double res = 0;

    assert(argc == 2);

    char * empty_string = malloc(sizeof(char) * map_size);
    memset(empty_string, '\0', map_size);

    const char *filename = argv[1];

    FILE *file = fopen(filename, "r");
    int map_fd = shm_open(BackingFile, O_RDWR, AccessPerms);
    if(map_fd < 0){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    caddr_t memptr = mmap(
            NULL,
            map_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            map_fd,
            0
            );
    if(memptr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    sem_t *semptr = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);


    if(semptr == SEM_FAILED){
        perror("semptr");
        exit(EXIT_FAILURE);
    }

    if(sem_wait(semptr) != 0){
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }


    while(fscanf(file,"%lf%c", &a, &c) != EOF) {
        res += a;
        if(c == '\n') {
            send_parent(memptr, semptr, empty_string, res);
            res = 0.;
            continue;
        }
    }

    if(!(res < eps && res > -eps))
        send_parent(memptr, semptr, empty_string, res);


    while(true) {
        if (sem_wait(semptr) == 0) {
            if(strcmp(memptr, empty_string) != 0) {
                if (sem_post(semptr) != 0) {
                    perror("sem_post");
                    exit(EXIT_FAILURE);
                }
                continue;
            }
            memptr[0] = EOF;
            if (sem_post(semptr) != 0) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }
            break;
        } else {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
    }

    if(munmap(memptr, map_size) != 0){
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    sem_close(semptr);

    return EXIT_SUCCESS;
}


//
// Created by sakost on 27.11.2020.
//

#ifndef INC_4_LAB_SHRMEM_H
#define INC_4_LAB_SHRMEM_H


#include <fcntl.h>

const size_t map_size = 4096;

const char * BackingFile = "os_lab4.back";
const char * SemaphoreName = "os_lab4.semaphore";
unsigned AccessPerms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;


#endif //INC_4_LAB_SHRMEM_H

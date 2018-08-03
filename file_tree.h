//
// Created by Matthew Bachstein on 7/6/18.
//

#ifndef LCIO_FILE_TREE_H
#define LCIO_FILE_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <search.h>
#include "lcio.h"

#define MAX_FNAME_SIZE 64

static const char alphanum[] = "abcdefghijklmnopqrstuvwxyz"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "0123456789";


struct file_entry{
    char* fname; // this will be the key,
    off_t size; // if size is 0, it is a dir.
};

off_t age_file_system(lcio_job_t* job, lcio_dist_t* dist);


#endif //LCIO_FILE_TREE_H

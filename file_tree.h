/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the LCIO project.
 ****************************************************************************/

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

#define MAX_FNAME_SIZE 61

static const char alphanum[] = "abcdefghijklmnopqrstuvwxyz"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "0123456789";


struct file_entry{
    char* fname; // this will be the key,
    off_t size; // if size is 0, it is a dir.
};

off_t age_file_system(lcio_job_t* job, lcio_dist_t* dist);


#endif //LCIO_FILE_TREE_H

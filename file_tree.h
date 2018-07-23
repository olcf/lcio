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

const float g_dist_real[26] = {45.00, 13.00, 21.00, 16.00, 11.00,
                               5.00, 25.00, 11.00, 26.00, 22.00,
                               49.00, 45.00, 6.00, 12.00, 18.00,
                               1.00,  0.50,  0.50, 0.50, 0.01,
                               0.00, 0.00, 0.00, 0.00, 0.00, 0.00};

const float g_dist_test[26] = {45.00, 13.00, 21.00, 16.00, 11.00,
                               5.00, 25.00, 11.00, 26.00, 22.00,
                               49.00, 45.00, 6.00, 0.00, 0.00,
                               0.00,  0.00,  0.00, 0.00,  0.00,
                               0.00, 0.00, 0.00, 0.00, 0.00, 0.00};

const char *sizes[26] = {"4k","8k", "16k", "32k", "64k",
                         "128k", "256k","512k", "1m", "2m",
                         "4m", "16m", "32m","64m","128m",
                         "256m","512m","1g", "4g", "64g",
                         "128g", "256g", "512g", "1t", "4t", "8t"};

struct file_entry{
    char* fname; // this will be the key,
    long long int size; // if size is 0, it is a dir.
};




#endif //LCIO_FILE_TREE_H

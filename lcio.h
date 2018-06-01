//
// Created by Matthew Bachstein on 5/23/18.
//

#ifndef LCIO_LCIO_H
#define LCIO_LCIO_H
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <limits.h>
#include <errno.h>
#include <mpi.h>
#include <string.h>

#include "conf_parser.h"

#define ERR(MSG) do {                           \
    fprintf(stderr, "%s:%d -- ERROR:%s (%d: %s)\n", \
            __FILE__, __LINE__, MSG, errno, strerr(errno));\
    exit(1); \
    } while(0)

#define ELOCAL(MSG) do {\
        fprintf(stderr, "%s:%d -- LOCAL ERROR:%s\n",            \
                __FILE__, __LINE__, MSG);                       \
        exit(1);                                                \
    } while(0)


#define FILE_WARN(FN, S1, S2) fprintf( \
        stderr, "%s:%d -- WARN: file size mismatch (%s -- %lld :: %ld)\n",\
        __FILE__,__LINE__,FN, S1, S2);\
/*
 * lcio_param_t and lcio_job_t describe the global parameters
 * that are needed for the entire run and the parameters for a
 * specific job in the run. That is, lcio_param_t is global for all
 * processes, and lcio_job_t is local to a particular mpi process
 * group.
 * If this is modified, need to modify [params.c]
 */
typedef struct lcio_engine lcio_engine_t;
typedef struct lcio_job {
    char lib_name[32];
    char tmp_dir[32];
    char type[16];
    char engine[8];
    int num_pes;
    int num_files;
    unsigned long long blk_sz;
    int fsync;
    char mode;
    void* lib_handle;
    lcio_engine_t* ioengine;

} lcio_job_t;

typedef struct lcio_param {
    int num_pes;
    int num_jobs;
    lcio_job_t** jobs;
} lcio_param_t;


/*
 * lcio_engine_t describes the operations that lcio can perform.
 * These are function pointers that can be initialized with any
 * of the supported IO libraries (POSIX, MPI_IO, AIO) with the
 * goal of creating a uniform interface for the function calls.
 */
typedef struct lcio_engine {
    char *name;
    void *(*create)(char*, lcio_job_t*);
    void *(*open)(char*, lcio_job_t*);
    void (*close)(int*, lcio_job_t*);
    void (*remove)(char*, lcio_job_t*);
    void *(*write)(const int*, lcio_job_t*);
    void *(*read)(const int*, lcio_job_t*);
    void *(*stat)(char*, lcio_job_t*);
    void (*fsync)(const int*, lcio_job_t*);

} lcio_engine_t;

void file_test(lcio_job_t*);

lcio_param_t* fill_parameters(struct conf*);
void print_params(lcio_param_t*);

double get_time(void);
double elapsed_time(double, double);


#endif //LCIO_LCIO_H

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


double get_time(void);
double elapsed_time(double, double);


/*
 * lcio_param_t and lcio_job_t describe the global parameters
 * that are needed for the entire run and the parameters for a
 * specific job in the run. That is, lcio_param_t is global for all
 * processes, and lcio_job_t is local to a particular mpi process
 * group.
 * If this is modified, need to modify [params.c]
 */
struct lcio_engine;
typedef struct lcio_job {
    char* engine;
    int num_pes;
    struct lcio_engine* ioengine;
} lcio_job_t;

typedef struct lcio_param {
    int num_pes;
    int num_jobs;
    lcio_job_t** jobs;
} lcio_param_t;

lcio_param_t* fill_parameters(struct conf*);
void print_params(lcio_param_t*);

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
    void (*close)(char*, lcio_job_t*);
    void (*delete)(char*, lcio_job_t*);
    int  (*stat)(void*);
    void (*fsync)(void*);

} lcio_engine_t;


#endif //LCIO_LCIO_H

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
#include "lcio_math.h"

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
    } while(0);


#define FILE_WARN(FN, S1, S2) fprintf( \
        stderr, "%s:%d -- WARN: file size mismatch " \
                "(%s -- %lld :: %ld)\n",\
        __FILE__,__LINE__,FN, S1, S2);

#define LOG(MSG) \
        fprintf(stdout, "%s\n", MSG);\
        fflush(stdout);

/*
 * lcio_param_t and lcio_job_t describe the global parameters
 * that are needed for the entire run and the parameters for a
 * specific job in the run. That is, lcio_param_t is global for all
 * processes, and lcio_job_t is local to a particular mpi process
 * group.
 * If this is modified, need to modify [params.c]
 */
typedef struct lcio_engine lcio_engine_t;
/*
 * IMPORTANT: The order here matters since this struct is
 * compressed into an MPI datatype for broadcasting.
 * the last three parameters (lib_name, lib_handle, ioengine)
 * are NOT read in by main, they are there for each process to
 * fill in as necessary. As such, the MPI Datatype for this
 * struct only takes the first 8 fields.
 * IF THIS IS MODIFIED, MODIFY THE DATATYPE DEFINITION AT
 * main.c:[40-65]
 */
typedef struct lcio_job {
    char tmp_dir[32];
    char type[16];
    char engine[8];
    int num_pes;
    int num_files;
    unsigned long long blk_sz;
    int fsync;
    int depth;
    float mean;
    float stdev;
    char mode;
    //======Datatype ends here=============
    int num_runs;
    MPI_Comm group_comm;
    int num_files_per_proc;
    char lib_name[32];
    void* lib_handle;
    lcio_results_t* job_timings;
    lcio_results_t* job_results;
    lcio_engine_t* ioengine;

} lcio_job_t;

typedef struct lcio_param {
    int num_pes;
    int num_jobs;
    int num_runs;
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


static char* prefix_g ="lcio_tmpf";

void file_test_full(lcio_job_t *);
void file_test_light(lcio_job_t *);

lcio_param_t* fill_parameters(struct conf*);
void print_params(lcio_param_t*);

double get_time(void);
double elapsed_time(double, double);
void execute_job(lcio_job_t* job);


#endif //LCIO_LCIO_H

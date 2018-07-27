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
    MPI_Abort(MPI_COMM_WORLD); \
    exit(1); \
    } while(0)

#define ELOCAL(MSG) do {\
        fprintf(stderr, "%s:%d -- LOCAL ERR:%s\n",            \
                __FILE__, __LINE__, MSG);               \
        MPI_Abort(MPI_COMM_WORLD, 1);                      \
        exit(1);                                        \
    } while(0);


#define FILE_WARN(FN, S1, S2) fprintf( \
        stderr, "%s:%d -- WARN: file size mismatch " \
                "(%s -- %lld :: %ld)\n",\
        __FILE__,__LINE__,FN, S1, S2);

#define MPI_FAIL(MSG, RNK)\


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
 * The items below the comment
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
    unsigned long long buf_sz;
    int fsync;
    int clean;
    int ops;
    int overlap;
    int epochs;
    char mode;
    //======Datatype ends here=============
    int job_number;
    int num_runs;
    char* buffer;
    MPI_Comm group_comm;
    MPI_Info info;
    int num_files_per_proc;
    char lib_name[32];
    void* lib_handle;
    lcio_results_t* job_timings;
    lcio_results_t* job_results;
    lcio_engine_t* ioengine;

} lcio_job_t;

typedef struct lcio_stage {
    int num_jobs_in_stage;
    int jobs_in_stage[31];
} lcio_stage_t;

typedef struct lcio_param {
    int num_pes;
    int num_jobs;
    int num_runs;
    int num_stages;
    lcio_stage_t** stages;
    lcio_job_t** jobs;
} lcio_param_t;


/*
 * lcio_engine_t describes the operations that lcio can perform.
 * These are function pointers that can be initialized with any
 * of the supported IO libraries (POSIX, MPI_IO, ) with the
 * goal of creating a uniform interface for the function calls.
 */
typedef struct lcio_engine {
    char *name;
    void* (*create)(char*, lcio_job_t*);
    void* (*open)(char*, lcio_job_t*);
    void  (*close)(void *, lcio_job_t *);
    void  (*remove)(char*, lcio_job_t *);
    void * (*write)(void *, lcio_job_t *, off_t flag);
    void* (*read)(void *, lcio_job_t *);
    void* (*stat)(void *, lcio_job_t *);
    void  (*fsync)(void *, lcio_job_t *);

} lcio_engine_t;


static char* prefix_g ="lcio_tmpf";

void file_test_full(lcio_job_t *);
void file_test_light(lcio_job_t *);

lcio_param_t* fill_parameters(struct conf*);
void print_params(lcio_param_t*);

double get_time(void);
double elapsed_time(double, double);
void execute_job(lcio_job_t* job);
void execute_aging(lcio_job_t* job);
void report_job_stats(lcio_job_t*);
void process_bandwidths(lcio_job_t*);
#endif //LCIO_LCIO_H

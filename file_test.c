/*
 * file_test.c
 *
 * Handles tests based around file metadata,
 * i.e. creation, removal, read, stat, etc
 */

#include "lcio.h"
#include <dlfcn.h>
#include <sys/stat.h>

void print_log(double t, char* op){
    printf("operation: %s\n  elapsed time: %.2lf usec\n\n", op, t);
}

void lcio_register_engine(lcio_job_t *job){
    char lib[64];
    char* error;
    void* handle;
    void (*register_ioengine)(lcio_job_t*);

    sprintf(lib, "lib%s.so", job->engine);
    job->lib_name = strdup(lib);
    handle = dlopen(lib, RTLD_LAZY);
    if(!handle){
        fputs(dlerror(), stderr);
        exit(1);
    }

    job->lib_handle = handle;
    register_ioengine = dlsym(handle, "register_ioengine");
    if((error = dlerror()) != NULL){
        fputs(error, stderr);
        exit(1);
    }
    register_ioengine(job);
 }

void lcio_create(lcio_job_t* job){
    int i;
    int* fd;
    char file[64];
    char* prefix = "/lcio_tmpf.";

    for(i=0; i < job->num_files; ++i){
        sprintf(file, "%s%s%d",job->tmp_dir, prefix, i);
        fd = (int*) job->ioengine->create(file, job);
        job->ioengine->close(fd, job);
    }
}

void lcio_write(lcio_job_t* job){
    int* fd;
    int i;
    char file[64];
    char buf[job->blk_sz];
    char* prefix = "/lcio_tmpf.";

    for(i=0; i < job->num_files; ++i){
        sprintf(file, "%s%s%d",job->tmp_dir, prefix, i);
        fd = (int*) job->ioengine->open(file, job);
        job->ioengine->write(fd, job);
        if(job->fsync){
            job->ioengine->fsync(fd, job);
        }
        job->ioengine->close(fd, job);
    }

}

void lcio_remove(lcio_job_t* job){
    int i;
    int* fd;
    char file[64];
    char* prefix = "lcio_tmpf.";

    for(i=0; i < job->num_files; ++i){
        sprintf(file, "%s%s%d",job->tmp_dir, prefix, i);
        job->ioengine->remove(file, job);
    }
}


void lcio_setup(lcio_job_t* job){
    lcio_register_engine(job);
    job->fd_array = malloc(sizeof(int) * job->num_files);
    mkdir(job->tmp_dir, S_IRWXU | S_IRWXG);
}


void file_test(lcio_job_t* job){

    lcio_setup(job);
    double times[3];
    double t1, t2;
    double final = 0.0;
    int i;

    t1 = get_time();
    lcio_create(job);
    t2 = get_time();
    times[0] = elapsed_time(t2,t1);
    print_log(times[0], "create");

    t1 = get_time();
    lcio_write(job);
    t2 = get_time();
    times[1] = elapsed_time(t2,t1);
    print_log(times[1], "write");

    t1 = get_time();
    lcio_remove(job);
    t2 = get_time();
    times[2] = elapsed_time(t2,t1);
    print_log(times[2], "remove ");

    for(i=0; i <= 2;++i) final += times[i];
    print_log(final, "final");

}
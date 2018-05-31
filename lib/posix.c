#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "lcio.h"

/*
 * POSIX IO engine.
 * Any interface must fill out a lcio_engine_t
 * struct and implement the
 * 'void register_ioengine(lcio_job_t*)' function
 */

void* posix_create(char* fn, lcio_job_t* job){
    int* fd;
    //int flags = O_CREAT | O_RDWR;
    fd = malloc(sizeof(int));
    *fd = creat(fn, 0644);
    return (void*)fd;
}

void* posix_open(char* fn, lcio_job_t* job){
    int* fd;
    int flags = O_RDWR;
    fd = malloc(sizeof(int));

    *fd = open(fn, flags, 0644);
    return (void*)fd;
}

void posix_close(int* fdes, lcio_job_t* job){

    close(*fdes);
    free(fdes);
}

void posix_delete(char* fn, lcio_job_t* job){
    unlink(fn);
}

void* posix_write(const int* fdes, lcio_job_t* job){
    ssize_t *rv;
    char buf[job->blk_sz];
    rv = malloc(sizeof(ssize_t));
    memset(buf, 'a', job->blk_sz);

    *rv = write(*fdes, buf, job->blk_sz);
    return (void*)rv;
}

void* posix_read(const int* fdes, lcio_job_t* job){
    ssize_t *rv;
    char buf[job->blk_sz];

    rv = malloc(sizeof(ssize_t));


    *rv = read(*fdes, buf, job->blk_sz);
    return (void*)rv;
}

void* posix_stat(char* fn, lcio_job_t* job){
    struct stat statbuf;
    int* err;

    err = malloc(sizeof(int));
    *err = stat(fn, &statbuf);
    if(statbuf.st_size != job->blk_sz){
        FILE_WARN(fn, statbuf.st_size , job->blk_sz);
    }
    return (void*)err;
}

void posix_fsync(const int* fdes, lcio_job_t* job){
    fsync(*fdes);
}


static lcio_engine_t posix_ioengine = {
    .name = "POSIX",
    .create = posix_create,
    .open = posix_open,
    .close = posix_close,
    .remove = posix_delete,
    .write = posix_write,
    .read = posix_read,
    .fsync = posix_fsync,
    .stat = posix_stat
};

void register_ioengine(lcio_job_t *job){
    job->ioengine = &posix_ioengine;
}





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
    unsigned int flags = O_CREAT | O_RDWR | O_TRUNC;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    fd = malloc(sizeof(int));
    *fd = open(fn, flags, mode);

    return (void*)fd;
}

void* posix_open(char* fn, lcio_job_t* job){
    int* fd;
    int flags = O_CREAT| O_APPEND| O_RDWR;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    fd = malloc(sizeof(int));
    *fd = open(fn, flags, mode);
    return (void*)fd;
}

void posix_close(void* fdes, lcio_job_t* job){

    close(*(int*)fdes);
    free(fdes);
}

void posix_delete(char* fn, lcio_job_t* job){
    unlink(fn);
}

void *posix_write(void *fdes, lcio_job_t *job, off_t flag) {
    ssize_t *rv;
    unsigned long long i;
    rv = malloc(sizeof(ssize_t));
    *rv = 0;
    if(flag == 0) {
        for (i = 0; i < job->blk_sz; i += job->buf_sz) {
            *rv += write(*(int *) fdes, job->buffer, job->buf_sz);
        }
    }
    else {
        unsigned long long count = flag / job->buf_sz;
        unsigned long long rem = flag % job->buf_sz;
        for(i = 0; i < count; i += 1) {
            *rv += write(*(int *) fdes, job->buffer, job->buf_sz);
        }
        *rv += write(*(int *) fdes, job->buffer, rem);
    }
    return (void*)rv;
}

void* posix_read(void* fdes, lcio_job_t* job){
    ssize_t *rv;
    unsigned long long i;
    rv = malloc(sizeof(ssize_t));
    *rv = 0;
    for (i = 0; i < job->blk_sz; i += job->buf_sz) {
        *rv += read(*(int *) fdes, job->buffer, job->buf_sz);
    }
    return (void*)rv;
}

void* posix_stat(void* fn, lcio_job_t* job){
    struct stat statbuf;
    int* err;

    err = malloc(sizeof(int));
    *err = stat((char*)fn, &statbuf);
    //if(statbuf.st_size != job->blk_sz && job->mode == 'U'){
    //    FILE_WARN((char*)fn, statbuf.st_size , job->blk_sz);
    //}
    return (void*)err;
}

void posix_fsync(void* fdes, lcio_job_t* job){
    fsync(*(int*)fdes);
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





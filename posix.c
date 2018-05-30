#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "lcio.h"

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

void posix_fsync(int* fdes, lcio_job_t* job){

}


static lcio_engine_t posix_ioengine = {
    .name = "POSIX",
    .create = posix_create,
    .open = posix_open,
    .close = posix_close,
    .remove = posix_delete,
    .write = posix_write,
    .read = posix_read
};

void register_ioengine(lcio_job_t *job){
    job->ioengine = &posix_ioengine;
}





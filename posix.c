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

void posix_close(int fdes, lcio_job_t* job){
    close(fdes);
}

void posix_delete(char* fn, lcio_job_t* job){
    unlink(fn);
}

void* posix_write(int fdes, void* buf, size_t nb){
    ssize_t *rv;
    *rv = write(fdes, buf, nb);
    return (void*)rv;
}

void* posix_read(int fdes, void* buf, size_t nb){
    ssize_t *rv;
    *rv = read(fdes, buf, nb);
    return (void*)rv;
}

static lcio_engine_t ioengine = {
    .name = "POSIX",
    .create = posix_create,
    .open = posix_open,
    .close = posix_close,
    .delete = posix_delete,
    .write = posix_write,
    .read = posix_read
};

void register_ioengine(lcio_job_t *job){
    job->ioengine = &ioengine;
}





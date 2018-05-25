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


static lcio_engine_t ioengine = {
    .name = "POSIX",
    .create = posix_create,
    .open = posix_open,
    .close = posix_close,
    .delete = posix_delete

};





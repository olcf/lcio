//
// Created by Matthew Bachstein on 6/14/18.
//

#include <mpi.h>
#include "lcio.h"

void* mpiio_create(char* fn, lcio_job_t* job){
    MPI_File* fh;
    MPI_Comm comm;
    fh = malloc(sizeof(MPI_File));
    int mode = MPI_MODE_CREATE | MPI_MODE_RDWR;

    /*
     * have to handle unique accesses vs shared file.
     */
    comm = (job->mode == 'S') ? job->group_comm : MPI_COMM_SELF;

    MPI_File_open(comm, fn, mode, job->info, fh);

    return (void*) fh;
}

void* mpiio_open(char* fn, lcio_job_t* job){
    MPI_File* fh;
    MPI_Comm comm;
    fh = malloc(sizeof(MPI_File));
    int mode = MPI_MODE_APPEND | MPI_MODE_RDWR;

    comm = (job->mode == 'S') ? job->group_comm : MPI_COMM_SELF;

    MPI_File_open(comm, fn, mode, job->info, fh);
    return (void*) fh;
}

void mpiio_close(void* fdes, lcio_job_t* job){
    MPI_File_close((MPI_File*)fdes);
    free(fdes);
}

void mpiio_delete(char* fn, lcio_job_t* job){
    MPI_File_delete(fn, job->info);
}

void* mpiio_write(void* fdes, lcio_job_t* job){
    MPI_Status* stat;
    stat = malloc(sizeof(MPI_Status));
    MPI_File_write(*(MPI_File*)fdes, job->buffer,
                  (int)job->blk_sz,
                  MPI_CHAR,
                  stat);
    return (void*) stat;
}

void* mpiio_read(void* fdes, lcio_job_t* job){
    MPI_Status* stat;
    stat = malloc(sizeof(MPI_Status));
    MPI_File_read(*(MPI_File*)fdes, job->buffer,
                  (int)job->blk_sz,
                  MPI_CHAR,
                  stat);
    return (void*)stat;
}

void* mpiio_stat(void* fdes, lcio_job_t* job){

    MPI_File* fh;
    MPI_Offset* sz;

    fh =(MPI_File*) mpiio_open((char*)fdes, job);
    sz = malloc(sizeof(MPI_Offset));

    MPI_File_get_size(*fh, sz);
    mpiio_close(fh, job);

    return (void*)sz;
}

void mpiio_fsync(void* fdes, lcio_job_t* job){
    MPI_File_sync(*(MPI_File*)fdes);
}



static lcio_engine_t mpiio_ioengine = {
    .name = "MPIIO",
    .create = mpiio_create,
    .open = mpiio_open,
    .close = mpiio_close,
    .remove = mpiio_delete,
    .write = mpiio_write,
    .read = mpiio_read,
    .fsync = mpiio_fsync,
    .stat = mpiio_stat
};

void register_ioengine(lcio_job_t* job){
    job->ioengine = &mpiio_ioengine;
}
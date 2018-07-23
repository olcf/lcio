//
// Created by Matthew Bachstein on 7/6/18.
//

#include "file_tree.h"
#include "lcio.h"
#include <dlfcn.h>


char* process_unique_dir(lcio_job_t* job){
    char* u_dir;
    char* tmp;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    tmp = malloc(sizeof(char)*16);
    u_dir = malloc(sizeof(char) * 256);
    strcpy(u_dir, job->tmp_dir);
    sprintf(tmp, "/proc%d", rank);
    strcat(u_dir, tmp);
    free(tmp);

    return u_dir;
}

void register_engine(lcio_job_t *job){
    char lib[64];
    char* error;
    void* handle;
    void (*register_ioengine)(lcio_job_t*);

    sprintf(lib, "lib%s.so", job->engine);
    strcpy(job->lib_name, lib);
    handle = dlopen(lib, RTLD_NOW);
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

void setup_aging(lcio_job_t* job){

    register_engine(job);
    //job->fd_array = malloc(sizeof(int) * job->num_files);
    mkdir(job->tmp_dir, S_IRWXU | S_IRWXG);
}

void age_file_system(lcio_job_t* job){


}



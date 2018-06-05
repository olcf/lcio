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
    printf("operation: %s\n  elapsed time: %.2lf \n\n", op, t);
}

int lcio_filename_unique(char *file, char *dir, int i){
    int rank;
    int err;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    err = sprintf(file, "%s/%s.%d.%d", dir, prefix_g, i,rank);
    return err;
}

int lcio_filname_shared(char* file, char* dir){
    int err;
    err = sprintf(file, "%s/%s.shared", dir, prefix_g);
    return err;
}

int lcio_filename(char* file, lcio_job_t* job, int i){
    if(job->mode == 'U') return lcio_filename_unique(file, job->tmp_dir, i);
    if(job->mode == 'S') return lcio_filname_shared(file, job->tmp_dir);
    return -1;
}

int lcio_directory_name_unique(char* file, char* dir, int i, int depth){

}

void lcio_register_engine(lcio_job_t *job){
    char lib[64];
    char* error;
    void* handle;
    void (*register_ioengine)(lcio_job_t*);

    sprintf(lib, "lib%s.so", job->engine);
    strcpy(job->lib_name, lib);
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
    //char* prefix = "/lcio_tmpf.";

    for(i=0; i < job->num_files; ++i){
        lcio_filename(file, job, i);
        //sprintf(file, "%s%s%d",job->tmp_dir, prefix, i);
        fd = (int*) job->ioengine->create(file, job);
        job->ioengine->close(fd, job);
    }
}

void lcio_write(lcio_job_t* job){
    int* fd;
    int i;
    char file[64];
    char buf[job->blk_sz];
    //char* prefix = "/lcio_tmpf.";

    for(i=0; i < job->num_files; ++i){
        lcio_filename(file, job, i);
        //sprintf(file, "%s%s%d",job->tmp_dir, prefix, i);
        fd = (int*) job->ioengine->open(file, job);
        job->ioengine->write(fd, job);
        if(job->fsync){
            job->ioengine->fsync(fd, job);
        }
        job->ioengine->close(fd, job);
    }

}

void lcio_stat(lcio_job_t* job){
    int i;
    char file[64];
    //char* prefix = "/lcio_tmpf.";

    for(i=0; i < job->num_files; ++i){
        lcio_filename(file, job, i);
        //sprintf(file, "%s%s%d",job->tmp_dir, prefix, i);
        job->ioengine->stat(file, job);
    }
}

void lcio_remove(lcio_job_t* job){
    int i;
    char file[64];
    //char* prefix = "/lcio_tmpf.";

    for(i=0; i < job->num_files; ++i){
        lcio_filename(file, job, i);
        //sprintf(file, "%s%s%d",job->tmp_dir, prefix, i);
        job->ioengine->remove(file, job);
    }
    job->ioengine->remove(job->tmp_dir, job);
}


void lcio_setup_unique(lcio_job_t* job){
    lcio_register_engine(job);
    //job->fd_array = malloc(sizeof(int) * job->num_files);
    mkdir(job->tmp_dir, S_IRWXU | S_IRWXG);
}

void lcio_setup_shared(lcio_job_t* job){
    lcio_register_engine(job);
    //job->fd_array = malloc(sizeof(int) * job->num_files);
    mkdir(job->tmp_dir, S_IRWXU | S_IRWXG);

}

void lcio_setup(lcio_job_t* job){
    if(job->mode == 'U'){
        lcio_setup_unique(job);
        return;
    }
    if(job->mode == 'S'){
        lcio_setup_shared(job);
        return;
    }
}

void lcio_teardown(lcio_job_t* job){
    job->ioengine = NULL;
    dlclose(job->lib_handle);
    rmdir(job->tmp_dir);
}

/*
 * This is ok for now, but needs some work for
 * the MPI integration. in particular, needs a more
 * portable reporting system. Maybe make a 'statistics'
 * struct?
 *
 */

void file_complete_test(lcio_job_t* job){

    lcio_setup(job);
    double times[16];
    double t1, t2;
    double final = 0.0;
    int i = 0;
    int j;

    /*
    t1 = get_time();
    lcio_create(job);
    t2 = get_time();
    times[i] = elapsed_time(t2,t1);
    print_log(times[i], "create");
    i+=1;
*/
    t1 = get_time();
    lcio_write(job);
    t2 = get_time();
    times[i] = elapsed_time(t2,t1);
    print_log(times[i], "write");
    i+=1;

    t1 = get_time();
    lcio_stat(job);
    t2 = get_time();
    times[i] = elapsed_time(t2,t1);
    print_log(times[i], "stat ");
    i+=1;

    t1 = get_time();
    lcio_remove(job);
    t2 = get_time();
    times[i] = elapsed_time(t2,t1);
    print_log(times[i], "remove ");
    i+=1;

    lcio_teardown(job);
    for(j=0; j < i;++j) final += times[j];
    print_log(final, "final");

}

void file_metadata_test(lcio_job_t* job){

    lcio_setup(job);
    double times[16];
    double t1, t2;
    double final = 0.0;
    int i = 0;
    int j;

    t1 = get_time();
    lcio_write(job);
    t2 = get_time();
    times[i] = elapsed_time(t2,t1);
    print_log(times[i], "write");
    i+=1;

    t1 = get_time();
    lcio_stat(job);
    t2 = get_time();
    times[i] = elapsed_time(t2,t1);
    print_log(times[i], "stat ");
    i+=1;

    t1 = get_time();
    lcio_remove(job);
    t2 = get_time();
    times[i] = elapsed_time(t2,t1);
    print_log(times[i], "remove ");
    i+=1;

    lcio_teardown(job);
    for(j=0; j < i;++j) final += times[j];
    print_log(final, "final");

}
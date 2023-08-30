/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the LCIO project.
 ****************************************************************************/

//
// Created by Matthew Bachstein on 7/6/18.
//

#include "file_tree.h"
#include "lcio.h"
#include <dlfcn.h>
#include <time.h>

char* gen_name(){
    long len;
    do {
        len = random() % MAX_FNAME_SIZE;
    }while(len == 0);
    char* s = malloc(sizeof(char) * len);
    int i;
    for (i = 0; i < len; ++i) {
        s[i] = alphanum[random() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
    return s;
}

char* gen_name_static(){
    static long long file_num = 0;
    int rank;
    char* s = malloc(sizeof(char) * (MAX_FNAME_SIZE + 6));

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    sprintf(s, "pf%d_%lld", rank, file_num);
    file_num += 1;

    return s;
}

off_t gen_size(lcio_dist_t* dist){
    float* d = compute_dist(dist);
    int i;
    double x,y;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while(1) {
        x = drand48();
        i = (int) floor((dist->len) * x);
        y = (dist->len) * x - i;
        if(y < d[i]) break;
    }

    return convert_suffix(dist->size[i]);
}

void delete_entry(struct file_entry* entry){
    free(entry->fname);
    free(entry);
}

struct file_entry* create_entry(lcio_dist_t* dist){
    struct file_entry* rv;
    rv = malloc(sizeof(struct file_entry)+ 1);
    rv->fname = gen_name_static();
    rv->size = gen_size(dist);
    return rv;
}

char* process_dir(lcio_job_t *job){
    char* u_dir;
    char* tmp;
    int rank;

    MPI_Comm_rank(job->group_comm, &rank);
    tmp = malloc(sizeof(char)*16);
    u_dir = malloc(sizeof(char) * 256);
    strcpy(u_dir, job->tmp_dir);
    /*
     * overlap is the number of processes sharing one directory
     */
    sprintf(tmp, "/proc%d", rank / job->overlap);
    strcat(u_dir, tmp);
    free(tmp);

    return u_dir;
}

void register_engine(lcio_job_t *job){
    char lib[64];
    char* error;
    void* handle;
    void (*register_ioengine)(lcio_job_t*);

    sprintf(lib, "./lib%s.so", job->engine);
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

off_t file_tree_write(struct file_entry* file, lcio_job_t* job){
    int* fd;
    int* err;
    int count = 0;
    err = malloc(sizeof(int));

    fd = (int*) job->ioengine->create(file->fname, job);
    if(*fd<0) {
        perror("open");
        fprintf(stderr,"fname %s\n", file->fname);
    }
    do {
        err = (int*) job->ioengine->write(fd, job, file->size);
        ++count;
        if(*err == -1) {
            perror("file_tree_write");
            fprintf(stderr,"count %d: fname %s\n" ,count, file->fname);
        }
    }while (*err == -1 && count < 5);
    if(job->fsync){
        job->ioengine->fsync(fd, job);
    }
    job->ioengine->close(fd, job);
    return (off_t) *err;
}

void file_tree_delete(struct file_entry* file, lcio_job_t* job){
    job->ioengine->remove(file->fname, job);
}

off_t file_tree_update(struct file_entry* file, lcio_job_t* job, lcio_dist_t* dist){
    file_tree_delete(file, job);
    file->size = gen_size(dist);
    return file_tree_write(file, job);
}

void setup_aging(lcio_job_t* job){
    char* my_u_dir;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    register_engine(job);
    mkdir(job->tmp_dir, S_IRWXU | S_IRWXG);
    srand48(rank);
    srandom(rank);

    my_u_dir = process_dir(job);

    mkdir(my_u_dir, S_IRWXU | S_IRWXG);
    chdir(my_u_dir);
}

void teardown_aging(lcio_job_t* job, struct file_entry** files){
    char* my_u_dir;
    int err;

    my_u_dir = process_dir(job);

    if(job->clean == 1) {
        int i;
        for (i = 0; i < job->num_files_per_proc; i++) {
            file_tree_delete(files[i], job);
        }

        err = chdir("../..");
        err = rmdir(my_u_dir);
        err = rmdir(job->tmp_dir);
    }
    job->ioengine = NULL;
    dlclose(job->lib_handle);
}

void print_files(struct file_entry** files, int num){
    int i;

    for(i = 0; i < num; i++){
        printf("%s :: %lld\n", files[i]->fname, files[i]->size);
    }
}

off_t calc_sizes(struct file_entry** files, int num){
    int i;
    off_t total_sz = 0;
    for(i = 0; i < num; i++){
        total_sz += files[i]->size;
    }
    return total_sz;
}


off_t age_file_system(lcio_job_t* job, lcio_dist_t* dist){

    struct file_entry **files;
    int i, j, k;
    off_t accum = 0;
    int my_rank;
    MPI_Comm_rank(job->group_comm, &my_rank);

    setup_aging(job);
    // in local work directory now.
    // there are $overlap number of processes
    // sharing the same directory
    //create list of files
    files = malloc(sizeof(struct file_entry*) * job->num_files_per_proc);
    for(i=0; i < job->num_files_per_proc; i++){
        files[i] = create_entry(dist);
    }
    // write the initial set of files

    for(i = 0; i < job->num_files_per_proc; i++){
        accum += file_tree_write(files[i], job);
    }

    //printf("rank[%d] :: file_sizes == %lld\n", my_rank, calc_sizes(files, job->num_files_per_proc));

    // do a some number of ops
    // in total, we do ops * epochs number of operations
    // epochs are controlled by a barrier to force the system to settle before the next
    // round.
    for(k = 0; k < job->epochs; k++) {
        for (j = 0; j < job->ops; j++) {
            i = gen_rand_uniform(job->num_files_per_proc);
            //printf("selected file %d :: %s\n", i, files[i]->fname);
            accum += file_tree_update(files[i], job, dist);
        }
        //printf("rank[%d] :: file_sizes == %lld\n", my_rank, calc_sizes(files, job->num_files_per_proc));
        MPI_Barrier(job->group_comm);
    }

    printf("rank[%d]: total bytes written %lld\n", my_rank, accum);
    teardown_aging(job, files);
    return accum;
}



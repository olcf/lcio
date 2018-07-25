//
// Created by Matthew Bachstein on 7/6/18.
//

#include "file_tree.h"
#include "lcio.h"
#include <dlfcn.h>
#include <time.h>

char* gen_name(){
    long len = random() % MAX_FNAME_SIZE;
    char* s = malloc(sizeof(char) * len);
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[random() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
    return s;
}

off_t gen_size(){
    float* d = compute_dist();
    int i;
    double x,y;
    while(1) {
        x = drand48();
        i = (int) floor(25.0 * x) + 1;
        y = 25.0 * x + 1 - i;
        //printf("%lf : %d : %lf : %lf\n",x, i, y, d[i]);
        if(y < d[i]) break;
    }
    return convert_suffix(sizes[i]);
}

void delete_entry(struct file_entry* entry){
    free(entry->fname);
    free(entry);
}

struct file_entry* create_entry(){
    struct file_entry* rv;
    rv = malloc(sizeof(struct file_entry));
    rv->fname = gen_name();
    rv->size = gen_size();
    return rv;
}

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

off_t file_tree_write(struct file_entry* file, lcio_job_t* job){
    int* fd;
    int* err;
    int count = 0;
    err = malloc(sizeof(int));

    fd = (int*) job->ioengine->open(file->fname, job);
    if(*fd<0) perror("Error opening file");
    do {
        err = (int*) job->ioengine->write(fd, job, file->size);
        ++count;
    }while (*err == -1 && count < 10);
    if(job->fsync){
        job->ioengine->fsync(fd, job);
    }
    job->ioengine->close(fd, job);
    return (off_t) *err;
}

void file_tree_delete(struct file_entry* file, lcio_job_t* job){
    job->ioengine->remove(file->fname, job);
}

off_t file_tree_update(struct file_entry* file, lcio_job_t* job){
    file_tree_delete(file, job);
    file->size = gen_size();
    return file_tree_write(file, job);
}

void setup_aging(lcio_job_t* job){
    char* my_u_dir;

    register_engine(job);
    mkdir(job->tmp_dir, S_IRWXU | S_IRWXG);
    srand48(time(NULL));

    my_u_dir = process_unique_dir(job);

    mkdir(my_u_dir, S_IRWXU | S_IRWXG);
    chdir(my_u_dir);
}

void teardown_aging(lcio_job_t* job, struct file_entry** files){
    char* my_u_dir;

    my_u_dir = process_unique_dir(job);

    if(job->clean == 1) {
        int i;
        for (i = 0; i < job->num_files_per_proc; i++) {
            file_tree_delete(files[i], job);
        }
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


off_t age_file_system(lcio_job_t* job){

    struct file_entry **files;
    int i, j;
    off_t accum = 0;

    setup_aging(job);

    // in local work directory now.
    // remember, each MPI process has its own directory
    //create list of files
    files = malloc(sizeof(struct file_entry*) * job->num_files_per_proc);
    for(i=0; i < job->num_files_per_proc; i++){
        files[i] = create_entry();
    }
    // write the initial set of files
    print_files(files, job->num_files_per_proc);
    for(i = 0; i < job->num_files_per_proc; i++){
        accum += file_tree_write(files[i], job);
    }

    for(j = 0; j < job->epoch; j++){
        i = gen_rand_uniform(job->num_files_per_proc);
        printf("selected file %d :: %s\n", i, files[i]->fname);
        accum += file_tree_update(files[i], job);
    }

    printf("total bytes written %lld\n", accum);
    teardown_aging(job, files);
    return accum;
}





#include <stdio.h>
#include "conf_parser.h"
#include "lcio.h"


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    int my_rank;
    int grp_sz;
    int world_sz;
    int color;
    int i;
    MPI_Comm world_comm;
    MPI_Comm group_comm;
    MPI_Group my_group;

    lcio_job_t* myjob;
    lcio_param_t *params;
    int num_jobs;

    MPI_Comm_dup(MPI_COMM_WORLD, &world_comm);

    MPI_Aint cextent, iextent, ullextent, fextent, flb, clb, ilb, ulllb;
    MPI_Type_get_extent(MPI_CHAR, &clb, &cextent);
    MPI_Type_get_extent(MPI_INT, &ilb, &iextent);
    MPI_Type_get_extent(MPI_UNSIGNED_LONG_LONG, &ulllb, &ullextent);
    MPI_Type_get_extent(MPI_FLOAT, &flb, &fextent);


    MPI_Comm_size(world_comm, &world_sz);
    MPI_Comm_rank(world_comm, &world_rank);

    /*
     * NOTE: this does not pack the lib_handle and ioengine
     * variables in lcio_job_t. These two are process specific,
     * and do not need to be broadcast.
     */
    MPI_Datatype MPI_LCIO_JOB;
    const int count = 11;
    int blens[count] = {32,16,8,1,1,1,1,1,1,1,1};
    MPI_Datatype array_of_types[count] =
            {MPI_CHAR, MPI_CHAR, MPI_CHAR,
             MPI_INT, MPI_INT,
             MPI_UNSIGNED_LONG_LONG,
             MPI_INT,MPI_INT, MPI_FLOAT, MPI_FLOAT,
             MPI_CHAR};
    MPI_Aint disps[count];

    /* displacements in struct */
    disps[0] = (MPI_Aint) 0; //tmp_dir
    disps[1] = cextent * 32; //type
    disps[2] = disps[1] + (cextent * 16); //engine
    disps[3] = disps[2] + (cextent * 8); //num_pes
    disps[4] = disps[3] + iextent; //num_files
    disps[5] = disps[4] + iextent; //blk_sz
    disps[6] = disps[5] + ullextent; //fsync
    disps[7] = disps[6] + iextent; //depth
    disps[8] = disps[7] + iextent; //mean
    disps[9] = disps[8] + fextent; //stddev
    disps[10] = disps[9] + fextent; //mode

    /*
     * Create datatype for lcio_job_t
     */
    MPI_Type_create_struct(count, blens, disps, array_of_types, &MPI_LCIO_JOB);
    MPI_Type_commit(&MPI_LCIO_JOB);


    /*
     * Root's (rank=0) responsibility is to
     * read the configurations and distribute out the
     * global information. For now, this is just the number of jobs
     * since that will be needed for creating the correct number of groups.
     */
    if( world_rank == 0) {
        char *name;
        struct conf *cfg;
        name = argv[1];
        cfg = parse_conf_file(name);
        params = fill_parameters(cfg);
        //num_jobs = params->num_jobs;
        //myjob = params->jobs[0];
        print_cfg(cfg);
    } else {
        params = malloc(sizeof(lcio_param_t));
    }

    /*
     * sorta nasty but other, more slick methods kept giving
     * me segfaults. Likely due to how the MPI_datatype isnt
     * a full struct the
     */
    MPI_Bcast(&(params->num_jobs), 1, MPI_INT, 0, world_comm);

    if(world_rank != 0){
        params->jobs = malloc(sizeof(lcio_job_t) * params->num_jobs);
    }

    MPI_Barrier(world_comm);
    for(i = 0; i < params->num_jobs; ++i){
        if(world_rank == 0) {myjob = params->jobs[i];}
        else { myjob = malloc(sizeof(lcio_job_t));}

        MPI_Bcast(myjob, 1, MPI_LCIO_JOB, 0, world_comm);
        if(world_rank != 0) params->jobs[i] = myjob;
    }
    MPI_Barrier(world_comm);

    color = world_rank % params->num_jobs;
    MPI_Comm_split(world_comm, color, world_rank, &group_comm);

    MPI_Comm_size(group_comm, &grp_sz);
    MPI_Comm_rank(group_comm, &my_rank);

    myjob = params->jobs[color];
    myjob->group_comm = group_comm;
    myjob->num_files_per_proc = myjob->num_files / grp_sz;

    printf("recved\n");
    printf("==============\n");
    printf(" %d::%d: tmp_dir:%s\n",my_rank, world_rank,myjob->tmp_dir);
    printf(" %d::%d: type:%s\n", my_rank,world_rank,myjob->type);
    printf(" %d::%d: engine:%s\n", my_rank,world_rank,myjob->engine);
    printf(" %d::%d: pes:%d\n",my_rank,world_rank, myjob->num_pes);
    printf(" %d::%d: num_files:%d\n",my_rank, world_rank,myjob->num_files);
    printf(" %d::%d: blk_sz:%lld\n",my_rank,world_rank, myjob->blk_sz);
    printf(" %d::%d: fsync:%d\n",my_rank, world_rank,myjob->fsync);
    printf(" %d::%d: mode:%c\n",my_rank,world_rank,myjob->mode);
    printf(" %d::%d: mean:%f\n", my_rank, world_rank,myjob->mean);
    printf(" %d::%d: stdev:%f\n", my_rank, world_rank,myjob->stdev);
    printf(" %d::%d: depth:%d\n", my_rank, world_rank,myjob->depth);
    fflush(stdout);


    if(!strcmp(myjob->type, "complete"))file_complete_test(myjob);
    if(!strcmp(myjob->type, "metadata"))file_metadata_test(myjob);
    MPI_Finalize();
    exit(0);
}

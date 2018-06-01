

#include <stdio.h>
#include "conf_parser.h"
#include "lcio.h"


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int my_rank;
    int comm_sz;
    MPI_Comm group_comm;
    lcio_job_t* myjob = malloc(sizeof(lcio_job_t));

    int num_jobs;

    MPI_Aint cextent, iextent, ullextent, clb, ilb, ulllb;
    MPI_Type_get_extent(MPI_CHAR, &clb, &cextent);
    MPI_Type_get_extent(MPI_INT, &ilb, &iextent);
    MPI_Type_get_extent(MPI_UNSIGNED_LONG_LONG, &ulllb, &ullextent);

    /*
     * NOTE: this does not pack the lib_handle and ioengine
     * variables in lcio_job_t. These two are process specific,
     * and do not need to be broadcast.
     */
    MPI_Datatype MPI_LCIO_JOB;
    const int count = 9;
    int blens[count] = {32,32,16,8,1,1,1,1,1};
    MPI_Datatype array_of_types[count] =
            {MPI_CHAR, MPI_CHAR, MPI_CHAR, MPI_CHAR,
             MPI_INT, MPI_INT,
             MPI_UNSIGNED_LONG_LONG,
             MPI_INT, MPI_CHAR};
    MPI_Aint disps[count];

    /* displacements in struct */
    disps[0] = (MPI_Aint) 0; //lib_name
    disps[1] = cextent * 32; //tmp_dir
    disps[2] = disps[1] + (cextent * 32); //type
    disps[3] = disps[2] + (cextent * 16); //engine
    disps[4] = disps[3] + (cextent * 8); //num_pes
    disps[5] = disps[4] + iextent; //num_files
    disps[6] = disps[5] + iextent; //blk_sz
    disps[7] = disps[6] + ullextent; //fsync
    disps[8] = disps[7] + iextent; //mode


    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

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
    if( my_rank == 0) {
        char *name;
        struct conf *cfg;
        lcio_param_t *params;
        name = argv[1];
        cfg = parse_conf_file(name);
        params = fill_parameters(cfg);
        num_jobs = params->num_jobs;
        myjob = params->jobs[0];
        //print_cfg(cfg);
    }
    MPI_Bcast(&num_jobs, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(myjob, 1, MPI_LCIO_JOB, 0, MPI_COMM_WORLD);

    if(my_rank == 1){
        printf("recved");
        printf("==============\n");
        printf(" lib_name:%s\n", myjob->lib_name);
        printf(" tmp_dir:%s\n", myjob->tmp_dir);
        printf(" type:%s\n", myjob->type);
        printf(" engine:%s\n", myjob->engine);
        printf(" pes:%d\n", myjob->num_pes);
        printf(" num_files:%d\n", myjob->num_files);
        printf(" blk_sz:%lld\n", myjob->blk_sz);
        printf(" fsync:%d\n", myjob->fsync);
        printf(" mode:%c\n",myjob->mode);
        fflush(stdout);
    }


    //file_test(params->jobs[0]);

    MPI_Finalize();
    exit(0);
}

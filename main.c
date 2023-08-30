/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the LCIO project.
 ****************************************************************************/



#include <stdio.h>
#include "conf_parser.h"
#include "lcio.h"


void print_help(){
    char msg[] = {
            "Usage: [MPI preamble] lcio [-h] -c CONFIG.INI -d DIST.INI\n"
            "\n"
            "Example: mpirun -np 2 ./lcio -c config_example.ini -d dist_example.ini\n\n"
            "LCIO: Lifecycle I/O tool\n\n"
            "  LCIO is a tool for rapidly aging large parallel filesystems. \n"
            "  LCIO requires two arguments files to be passed on the command line,\n"
            "  a configuration file that lays out how the run is supposed to be done, \n"
            "  and a distribution file that details the file sizes and counts.\n"
            "  LCIO was designed to work easily with the pcircle/fprof tool, which can generate\n"
            "  the distribution file. \n"
            "  LCIO requires MPI. (preferably > 3.0, works with 1.10 but some transient errors)\n"
            "  Please ensure that the number of processes match in the config file and in the mpirun call\n"
            "\n"
            "Flags: \n"
            "  -c, --config : configuration file for run\n"
            "  -d, --dist : file containing file size distribution. (section mut be named [dist])\n"
            "  -h, --help : print this message and exit with MPI_Abort()\n\n"
    };

    printf("%s", msg);
}

struct lcio_opts* parse_cli_args(int argc, char** argv){
    int i;
    struct lcio_opts *opts;

    if (argc <= 1) return NULL;

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
        return NULL;
    }

    opts = malloc(sizeof(struct lcio_opts));

    for(i = 1; i < argc; i += 1){

        if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0){
            i += 1;
            opts->cfg_fname = strdup(argv[i]);
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dist") == 0){
            i += 1;
            opts->dist_fname = strdup(argv[i]);
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
            free(opts);
            opts = NULL;
        }
    }

    return opts;
}

void check_config(lcio_param_t* params, int size){
    /*
     * Need to check and make sure that the user is
     * not asking for more in a stage than given with
     * mpirun
     */
    int nstage;
    int i;
    int njob;
    lcio_stage_t* mystage;
    lcio_job_t* job;
    int accum;

    for(nstage = 0; nstage < params->num_stages; nstage+=1) {
        mystage = params->stages[nstage];
        accum = 0;
        for (i = 0; i < mystage->num_jobs_in_stage; i++) {
            accum += params->jobs[mystage->jobs_in_stage[i]]->num_pes;
        }
        if(size < accum){
            fprintf(stderr, "Mismatch in processors.\n"
                            "Have %d ranks, %d ranks needed "
                            "for stage %d\n", size, accum, i);
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }
    }
}


int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int world_rank;
    int my_rank;
    int grp_sz;
    int world_sz;
    int err = 0;
    int color = MPI_UNDEFINED;
    int i, j, res;
    MPI_Comm world_comm;
    MPI_Comm group_comm;
    MPI_Group my_group;

    lcio_job_t* myjob;
    lcio_param_t *params;
    lcio_stage_t* mystage;
    lcio_dist_t* dist;
    int num_jobs;

    MPI_Datatype MPI_LCIO_JOB;
    MPI_Datatype  MPI_LCIO_STAGE;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Comm_dup(MPI_COMM_WORLD, &world_comm);

    MPI_Aint cextent, iextent, ullextent, fextent, flb, clb, ilb, ulllb;
    MPI_Type_get_extent(MPI_CHAR, &clb, &cextent);
    MPI_Type_get_extent(MPI_INT, &ilb, &iextent);
    MPI_Type_get_extent(MPI_UNSIGNED_LONG_LONG, &ulllb, &ullextent);
    MPI_Type_get_extent(MPI_FLOAT, &flb, &fextent);


    MPI_Comm_size(world_comm, &world_sz);
    MPI_Comm_rank(world_comm, &world_rank);
    /*
     * NOTE: this does not pack the trailing
     * variables in lcio_job_t. Those are process specific,
     * and do not need to be broadcast. See lcio.h:[60-77]
     */
    const int count = 14;
    int blens[14] = {128,16,8,1,1,1,1,1,1,1,1,1,1};
    MPI_Datatype array_of_types[14] =
            {MPI_CHAR, MPI_CHAR, MPI_CHAR,
             MPI_INT, MPI_INT,
             MPI_UNSIGNED_LONG_LONG,
             MPI_UNSIGNED_LONG_LONG,
             MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT,
             MPI_CHAR};
    MPI_Aint disps[14];

    /* displacements in struct */
    disps[0] = (MPI_Aint) 0; //tmp_dir
    disps[1] = cextent * 128; //type
    disps[2] = disps[1] + (cextent * 16); //engine
    disps[3] = disps[2] + (cextent * 8); //num_pes
    disps[4] = disps[3] + iextent; //num_files
    disps[5] = disps[4] + iextent; //blk_sz
    disps[6] = disps[5] + ullextent; //buf_sz
    disps[7] = disps[6] + ullextent; //fsync
    disps[8] = disps[7] + iextent; //clean
    disps[9] = disps[8] + iextent; //ops
    disps[10] = disps[9] + iextent; //overlap
    disps[11] = disps[10] + iextent; //epochs
    disps[12] = disps[11] + iextent; //ftrunc
    disps[13] = disps[12] + iextent; //mode

    /*
     * Create datatype for lcio_job_t
     */

    MPI_Type_create_struct(count, blens, disps, array_of_types, &MPI_LCIO_JOB);
    MPI_Type_commit(&MPI_LCIO_JOB);

    /*
     * Datatype for lcio_stage_t
     */
    const int c = 2;
    int blens2[2] = {1,31};
    MPI_Aint disps2[2];
    disps2[0] = (MPI_Aint) 0;
    disps2[1] = iextent;
    MPI_Datatype arr2[2] = {MPI_INT, MPI_INT};
    MPI_Type_create_struct(c,blens2, disps2, arr2, &MPI_LCIO_STAGE);
    MPI_Type_commit(&MPI_LCIO_STAGE);

    /*
     * Root's (rank=0) responsibility is to
     * read the configurations and distribute out the
     * global information. For now, this is just the number of jobs
     * since that will be needed for creating the correct number of groups.
     */
    if( world_rank == 0) {
        struct lcio_opts *opts;
        struct conf *cfg;
        struct conf *dist_cfg;

        opts = parse_cli_args(argc, argv);
        if(opts == NULL){
            print_help();
            MPI_Abort(world_comm, 0);
            exit(0);
        }

        cfg = parse_conf_file(opts->cfg_fname);
        if(cfg == NULL){
            perror("fopen");
            fprintf(stderr, "Configuration file not found.\n"
                            "Did you specify --config?\n");
            err = 1;
        }
        if(err != 1) {
            params = fill_parameters(cfg);

            check_config(params, world_sz);
        }

        dist_cfg = parse_conf_file(opts->dist_fname);
        if(dist_cfg == NULL){
            perror("fopen");
            fprintf(stderr, "Distribution file not found.\n"
                            "Did you specify --dist?\n");
            err = 1;
        }
        if (err != 1) dist = fill_dist(dist_cfg);
        //print_cfg(dist_cfg);
    } else {
        params = malloc(sizeof(lcio_param_t));
        dist = malloc(sizeof(lcio_dist_t));
    }

    MPI_Bcast(&err, 1, MPI_INT, 0, world_comm);
    if(err == 1){
        MPI_Finalize();
        exit(0);
    }

    /*
     * sorta nasty but other, more slick methods kept giving
     * me segfaults. Likely due to how the MPI_datatype isn't
     * a full struct.
     */
    MPI_Bcast(&(params->num_jobs), 1, MPI_INT, 0, world_comm);
    MPI_Bcast(&(params->num_stages), 1, MPI_INT, 0, world_comm);

    if(world_rank != 0){
        params->jobs = malloc(sizeof(lcio_job_t*) * params->num_jobs);
        params->stages = malloc(sizeof(lcio_stage_t*) * params->num_stages);
    }

    MPI_Barrier(world_comm);

    for(i=0; i < params->num_stages; i++){
        if(world_rank == 0){mystage = params->stages[i];}
        else { mystage = malloc(sizeof(lcio_stage_t));}

        MPI_Bcast(mystage, 1, MPI_LCIO_STAGE, 0, world_comm);

        if(world_rank != 0) params->stages[i] = mystage;
    }

    MPI_Barrier(world_comm);
    for(i = 0; i < params->num_jobs; ++i){
        if(world_rank == 0) {myjob = params->jobs[i];}
        else { myjob = malloc(sizeof(lcio_job_t));}

        MPI_Bcast(myjob, 1, MPI_LCIO_JOB, 0, world_comm);
        if(world_rank != 0) params->jobs[i] = myjob;
        params->jobs[i]->job_number = i;
    }
    MPI_Barrier(world_comm);

    /*
     * distribution of job config, now need to parcel out the
     * file count distribution
     */

    MPI_Bcast(&(dist->len), 1, MPI_INT, 0, world_comm);
    if(world_rank != 0){
        dist->size = malloc(dist->len * sizeof(char*));
        dist->array = malloc(dist->len * sizeof(float));
        for( i = 0; i < dist->len; i++) {
            dist->size[i] = malloc(sizeof(char) * 8);
        }
    }
    MPI_Barrier(world_comm);
    for (i = 0; i < dist->len; i++) {
        //NO NEED TO REFERENCE A CHAR ARRAY
        MPI_Bcast(dist->size[i], 8, MPI_CHAR, 0, world_comm);
        MPI_Bcast(&(dist->array[i]), 1, MPI_FLOAT, 0, world_comm);
    }
    MPI_Barrier(world_comm);
    /*
     * At this point, all pes have their work assignments and needed data.
     * Now, we start moving through the stages.
     * Each stage will setup/teardown its own group comm.
     */


    int nstage;
    for(nstage=0; nstage < params->num_stages; nstage++) {
        if(world_rank == 0){
            printf("\nStage %d\n", nstage);
        }

        mystage = params->stages[nstage];
        res = 0;
        for (i = 0; i < mystage->num_jobs_in_stage; i++) {
            res += params->jobs[mystage->jobs_in_stage[i]]->num_pes;
            if (world_rank < res) {
                color = i;
                break;
            }
        }
        MPI_Barrier(world_comm);
        /*
        if (color == -1) {
            fprintf(stderr, "ERROR: color failed: rank %d\n", world_rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }
        */
        MPI_Comm_split(world_comm, color, world_rank, &group_comm);
        //printf("stage:%d  wr:%d  gr: %d  color %d \n", nstage, world_rank, my_rank, color);


        if( color > -1) {
            MPI_Comm_size(group_comm, &grp_sz);
            MPI_Comm_rank(group_comm, &my_rank);
            myjob = params->jobs[mystage->jobs_in_stage[color]];
            myjob->group_comm = group_comm;
            myjob->num_files_per_proc = myjob->num_files / grp_sz;

            MPI_Barrier(world_comm);
            if(!(strcmp(myjob->type, "file_tree"))){
                execute_aging(myjob, dist);
            }
            else{
                myjob->num_runs = myjob->ops;
                execute_job(myjob);
            }
        }
        MPI_Barrier(world_comm);
    }
    MPI_Barrier(world_comm);
    MPI_Finalize();
    exit(0);
}

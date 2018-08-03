

#include <stdio.h>
#include "conf_parser.h"
#include "lcio.h"



int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    int my_rank;
    int grp_sz;
    int world_sz;
    int color =  -1;
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
    const int count = 13;
    int blens[13] = {32,16,8,1,1,1,1,1,1,1,1,1,1};
    MPI_Datatype array_of_types[13] =
            {MPI_CHAR, MPI_CHAR, MPI_CHAR,
             MPI_INT, MPI_INT,
             MPI_UNSIGNED_LONG_LONG,
             MPI_UNSIGNED_LONG_LONG,
             MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT,
             MPI_CHAR};
    MPI_Aint disps[13];

    /* displacements in struct */
    disps[0] = (MPI_Aint) 0; //tmp_dir
    disps[1] = cextent * 32; //type
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
    disps[12] = disps[11] + iextent; //mode

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
        char *name;
        struct conf *cfg;
        struct conf *dist_cfg;
        name = argv[1];
        cfg = parse_conf_file(name);
        if(cfg == NULL){
            perror("fopen");
            fprintf(stderr, "Configuration file not found.\n"
                            "Config file should be first command line argument.\n");
            MPI_Abort(world_comm, 1);
            exit(1);
        }
        params = fill_parameters(cfg);
        //print_cfg(cfg);

        name = argv[2];
        dist_cfg = parse_conf_file(name);
        if(dist_cfg == NULL){
            perror("fopen");
            fprintf(stderr, "Distribution file not found.\n"
                            "Dist file should be second command line argument.\n");
            MPI_Abort(world_comm, 1);
            exit(1);
        }
        dist = fill_dist(dist_cfg);
        //print_cfg(dist_cfg);
    } else {
        params = malloc(sizeof(lcio_param_t));
        dist = malloc(sizeof(lcio_dist_t));
    }

    /*
     * sorta nasty but other, more slick methods kept giving
     * me segfaults. Likely due to how the MPI_datatype isn't
     * a full struct.
     */
    MPI_Bcast(&(params->num_jobs), 1, MPI_INT, 0, world_comm);
    MPI_Bcast(&(params->num_runs), 1, MPI_INT, 0, world_comm);
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
        /*/
         * change this to use MPI_Group_incl_create
         */
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

        if (color == -1) {
            fprintf(stderr, "ERROR: color failed: rank %d\n", world_rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }
        MPI_Comm_split(world_comm, color, world_rank, &group_comm);

        MPI_Comm_size(group_comm, &grp_sz);
        MPI_Comm_rank(group_comm, &my_rank);
        printf("stage:%d  wr:%d  gr: %d  color %d \n", nstage, world_rank, my_rank, color);


        if( color > -1) {
            myjob = params->jobs[mystage->jobs_in_stage[color]];
            myjob->group_comm = group_comm;
            myjob->num_files_per_proc = myjob->num_files / grp_sz;
            myjob->num_runs = params->num_runs;

            MPI_Barrier(world_comm);
            if(!(strcmp(myjob->type, "file_tree"))){
                execute_aging(myjob, dist);
            }
            else{
                execute_job(myjob);
            }
        }
        MPI_Barrier(world_comm);
    }
    MPI_Barrier(world_comm);
    MPI_Finalize();
    exit(0);
}

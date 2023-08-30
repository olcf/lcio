/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the LCIO project.
 ****************************************************************************/



#include "lcio.h"
#include "conf_parser.h"


void print_params(lcio_param_t* params){
    fprintf(stderr, "Params:\n");
    fprintf(stderr, "  num_pes: %d\n", params->num_pes);
    fprintf(stderr, "  num_jobs: %d\n\n", params->num_jobs);

    int i;
    for(i=0; i < params->num_jobs; ++i){
        fprintf(stderr, "  Job%d:\n", i);
        fprintf(stderr, "    num_pes: %d\n", params->jobs[i]->num_pes);
        fprintf(stderr, "    engine: %s\n", params->jobs[i]->engine);
    }
}

void print_arr(lcio_stage_t* stage){
    int i;
    for( i = 0; i < stage->num_jobs_in_stage; i++){
        printf("job %d: %d\n", i, stage->jobs_in_stage[i]);
    }
}

void get_buf_sz(char* field, lcio_job_t* job, char* f){
    int base;
    unsigned long long exp;
    char scale;
    int err;
    if(field == NULL){
        if(!strcmp(f, "block")) job->blk_sz = 0;
        return;
    }

    err = sscanf(field,"%d%c", &base, &scale);
    if(err != 2){
        ELOCAL("Did not convert block size parameter");
    }

    switch(scale){
        case 'M':
        case 'm':
            exp = (1 << 20);
            break;
        case 'K':
        case 'k':
            exp = (1 << 10);
            break;
        default:
            exp = 1;
    }
    if(!strcmp(f, "block")) job->blk_sz = (base * exp);
    if(!strcmp(f, "buffer")) job->buf_sz = (base * exp);

}


void fill_stages(struct conf *cfg, lcio_param_t *params){
    struct section* sec;
    char buf[16];
    char* job_buf;
    int i, j;
    char* end;

    for(i = 0; i < params->num_stages; i++){
        sprintf(buf,"stage%d", i);
        sec = get_section(buf, cfg);
        params->stages[i] = malloc(sizeof(lcio_stage_t));
        params->stages[i]->num_jobs_in_stage = (int) strtol(get_attr("num_jobs", sec), &end, 10);
        job_buf = get_attr("jobs", sec);

        if(params->stages[i]->num_jobs_in_stage != (int)strlen(job_buf)) {
            ELOCAL("# jobs mismatch. Aborting");
        }

        for(j = 0; j < params->stages[i]->num_jobs_in_stage; j++){
            params->stages[i]->jobs_in_stage[j] = (int) job_buf[j] - '0';
        }
    }
}

/*
 * Need to change this function every time a parameter is
 * added to the config file
 */

void fill_jobs(struct conf *cfg, lcio_param_t* params){
    struct section* sec;
    char buf[16];
    int i;
    char* end;

    for(i=0; i < params->num_jobs; ++i){
        sprintf(buf,"job%d", i);
        sec = get_section(buf, cfg);
        params->jobs[i] = malloc(sizeof(lcio_job_t));

        strcpy(params->jobs[i]->engine, get_attr("engine", sec));
        strcpy(params->jobs[i]->type, get_attr("type", sec));
        strcpy(params->jobs[i]->tmp_dir, get_attr("tmp_dir", sec));

        params->jobs[i]->num_pes = (int)strtol(get_attr("mpi_num_pes", sec), &end, 10);
        params->jobs[i]->num_files = (int)strtol(get_attr("num_files", sec), &end, 10);
        params->jobs[i]->fsync = (int)strtol(get_attr("fsync", sec), &end, 10);
        params->jobs[i]->clean = (int)strtol(get_attr("clean", sec), &end, 10);
        if(get_attr("overlap", sec) != NULL)params->jobs[i]->overlap = (int)strtol(get_attr("overlap", sec), &end, 10);
        if(get_attr("epochs", sec) != NULL)params->jobs[i]->epochs = (int)strtol(get_attr("epochs", sec), &end, 10);
        if(get_attr("ops", sec) != NULL)params->jobs[i]->ops = (int)strtol(get_attr("ops", sec), &end, 10);
        if(get_attr("num_runs", sec) != NULL) params->jobs[i]->num_runs = (int)strtol(get_attr("num_runs", sec), &end, 10);
        if(get_attr("ftrunc", sec) != NULL) params->jobs[i]->ftrunc = (int)strtol(get_attr("ftrunc", sec), &end, 10);

        get_buf_sz(get_attr("buffer_size", sec), params->jobs[i], "buffer");
        get_buf_sz(get_attr("block_size", sec), params->jobs[i], "block");
        if(get_attr("mode", sec) != NULL) sscanf(get_attr("mode", sec), "%c", &(params->jobs[i]->mode));

    }
}


lcio_param_t* fill_parameters(struct conf *cfg){
    lcio_param_t* params;
    struct section* sec;
    char buf[8];
    int i;
    char* end;

    params = malloc(sizeof(lcio_param_t));

    if(NULL == (sec = get_section("setup", cfg))) {ELOCAL("no [setup] section");}
    params->num_jobs = (int)strtol(get_attr("num_jobs", sec), &end, 10);
    params->num_stages = (int)strtol(get_attr("num_stages", sec), &end, 10);

    params->jobs = malloc(sizeof(lcio_job_t*) * params->num_jobs);
    params->stages = malloc(sizeof(lcio_stage_t*) * params->num_stages);

    fill_jobs(cfg, params);
    fill_stages(cfg, params);
    return params;
}


lcio_dist_t* fill_dist(struct conf *cfg){
    lcio_dist_t* dist;
    struct section* sec;
    char buf[16];
    int i;
    char* end;

    dist = malloc(sizeof(lcio_dist_t));
    if(NULL == (sec = get_section("dist", cfg))) {ELOCAL("no [dist] section");}

    dist->len = sec->num;
    dist->size = get_keys(sec);
    dist->array = malloc(sizeof(float) * dist->len);
    for( i = 0; i < dist->len; i++){
        dist->array[i] = strtof(get_attr(dist->size[i], sec), &end);
    }
    return dist;
}



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

void get_buf_sz(char* field, lcio_job_t* job){
    int base;
    unsigned long long exp;
    char scale;
    int err;

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
            ELOCAL("Invalid scale (must be M or K)");
    }

    job->blk_sz = (base * exp);

}

/*
 * Need to change this function every time a parameter is
 * added to the config file
 */
lcio_param_t* fill_parameters(struct conf *cfg){
    lcio_param_t* params;
    struct section* sec;
    char buf[8];
    char block_buf[128];
    int i;
    char* end;

    params = malloc(sizeof(lcio_param_t));

    if(NULL == (sec = get_section("setup", cfg))) {ELOCAL("no [setup] section");}
    params->num_jobs = (int)strtol(get_attr("num_jobs", sec), &end, 10);
    params->num_pes = (int)strtol(get_attr("mpi_num_pes", sec), &end, 10);

    params->jobs = malloc(sizeof(lcio_job_t*) * params->num_jobs);

    /*
     * TODO: this needs to be cleaned up somewhat
     */
    for(i=0; i < params->num_jobs; ++i){
        sprintf(buf,"job%d", i);
        sec = get_section(buf, cfg);
        params->jobs[i] = malloc(sizeof(lcio_job_t));
        params->jobs[i]->num_pes = (int)strtol(get_attr("mpi_num_pes", sec), &end, 10);
        strcpy(params->jobs[i]->engine, get_attr("engine", sec));
        strcpy(params->jobs[i]->type, get_attr("type", sec));
        sscanf(get_attr("mode", sec), "%c", &(params->jobs[i]->mode));
        params->jobs[i]->num_files = (int)strtol(get_attr("num_files", sec), &end, 10);
        strcpy(params->jobs[i]->tmp_dir, get_attr("tmp_dir", sec));
        params->jobs[i]->fsync = (int)strtol(get_attr("fsync", sec), &end, 10);
        get_buf_sz(get_attr("block_size", sec), params->jobs[i]);
        params->jobs[i]->depth = (int)strtol(get_attr("depth", sec), &end, 10);
        params->jobs[i]->mean = strtof(get_attr("mean", sec), &end);
        params->jobs[i]->stdev = strtof(get_attr("stdev", sec), &end);

    }
    return params;
}

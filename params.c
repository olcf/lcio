

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
    unsigned long exp;
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

    job->blk_sz = base * exp;

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

    params = malloc(sizeof(lcio_param_t));

    if(NULL == (sec = get_section("setup", cfg))) {ELOCAL("no [setup] section");}
    params->num_jobs = atoi(get_attr("num_jobs", sec));
    params->num_pes = atoi(get_attr("mpi_num_pes", sec));

    params->jobs = malloc(sizeof(lcio_job_t*) * params->num_jobs);

    for(i=0; i < params->num_jobs; ++i){
        sprintf(buf,"job%d", i);
        sec = get_section(buf, cfg);
        params->jobs[i] = malloc(sizeof(lcio_job_t));
        params->jobs[i]->num_pes = atoi(get_attr("mpi_num_pes", sec));
        params->jobs[i]->engine = strdup(get_attr("engine", sec));
        params->jobs[i]->type = strdup(get_attr("type", sec));
        params->jobs[i]->mode = strdup(get_attr("mode", sec));
        params->jobs[i]->num_files = atoi(get_attr("num_files", sec));
        params->jobs[i]->tmp_dir = strdup(get_attr("tmp_dir", sec));
        params->jobs[i]->fsync = atoi(get_attr("fsync", sec));
        get_buf_sz(get_attr("block_size", sec), params->jobs[i]);
    }
    return params;
}

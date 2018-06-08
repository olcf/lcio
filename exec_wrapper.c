//
// Created by Matthew Bachstein on 6/7/18.
//

#include "lcio.h"
#include "lcio_math.h"

void execute_job(lcio_job_t* job){
    int rank;
    int group_sz;

    MPI_Comm_rank(job->group_comm, &rank);
    MPI_Comm_size(job->group_comm, &group_sz);

    /*
     * IOPS will be back calculated from the timings
     */
    job->job_timings = malloc(sizeof(lcio_results_t));
    job->job_timings->max_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_timings->min_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_timings->avg_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_timings->stddevs = calloc(TIME_ARR_SZ, sizeof(double));

    job->job_timings->raw_times = malloc(sizeof(double*) * job->num_runs);

    if(!strcmp(job->type, "metadata_full"))file_test_full(job);
    if(!strcmp(job->type, "metadata_light"))file_test_light(job);

    /*
     * This will be called twice, once for each process, then once
     * on the group root after the MPI_Gather.
     */
    process_times(job->job_timings, job->num_runs);


}
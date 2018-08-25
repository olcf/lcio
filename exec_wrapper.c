//
// Created by Matthew Bachstein on 6/7/18.
//

#include "lcio.h"
#include "file_tree.h"

void execute_aging(lcio_job_t* job, lcio_dist_t* dist){
    int rank;
    double t1,t2;
    MPI_Comm_rank(job->group_comm, &rank);
    job->buffer = calloc(job->buf_sz, sizeof(char));
    memset(job->buffer, 'c', job->buf_sz);

    if(rank == 0) {
        t1 = MPI_Wtime();
    }
    MPI_Barrier(job->group_comm);
    age_file_system(job, dist);
    MPI_Barrier(job->group_comm);
    if(rank == 0){
        t2 = MPI_Wtime();
        printf("total time: %lf\n", fabs(t2-t1));
    }
}


void execute_job(lcio_job_t* job){
    int rank;
    int group_sz;


    MPI_Comm_rank(job->group_comm, &rank);
    MPI_Comm_size(job->group_comm, &group_sz);

    MPI_Info_create(&(job->info));
    job->buffer = calloc(job->buf_sz, sizeof(char));
    memset(job->buffer, 'c', job->buf_sz);


    job->job_timings = malloc(sizeof(lcio_results_t));
    job->job_timings->max_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_timings->min_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_timings->avg_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_timings->variances = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_timings->raw_times = malloc(sizeof(double*) * job->num_runs);


    job->job_results = malloc(sizeof(lcio_results_t));
    job->job_results->max_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_results->min_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_results->avg_times = calloc(TIME_ARR_SZ, sizeof(double));
    job->job_results->variances = calloc(TIME_ARR_SZ, sizeof(double));
    if(rank == 0) {
        job->job_results->max_bandwidths = calloc(TIME_ARR_SZ, sizeof(double));
        job->job_results->min_bandwidths = calloc(TIME_ARR_SZ, sizeof(double));
        job->job_results->avg_bandwidths = calloc(TIME_ARR_SZ, sizeof(double));
        job->job_results->var_bandwidths = calloc(TIME_ARR_SZ, sizeof(double));
    };
    MPI_Barrier(job->group_comm);

    if(!strcmp(job->type, "rw"))file_test_full(job);
    if(!strcmp(job->type, "rw_light"))file_test_light(job);

    process_times(job->job_timings, job->num_runs);
    /*
     * Reduce can be used for the max and min arrays as it
     * operates elementwise.
     */
    MPI_Reduce(job->job_timings->max_times, job->job_results->max_times,
               TIME_ARR_SZ, MPI_DOUBLE, MPI_MAX, 0, job->group_comm);

    MPI_Reduce(job->job_timings->min_times, job->job_results->min_times,
               TIME_ARR_SZ, MPI_DOUBLE, MPI_MIN, 0, job->group_comm);

    /*
     * avg times use MPI_reduce to sum them, then divide each element by
     * the group_sz
     */
    MPI_Reduce(job->job_timings->avg_times, job->job_results->avg_times,
               TIME_ARR_SZ, MPI_DOUBLE, MPI_SUM, 0, job->group_comm);

    /*
     * variances sum, stddevs dont. use variances until the final step
     */
    MPI_Reduce(job->job_timings->variances, job->job_results->variances,
               TIME_ARR_SZ, MPI_DOUBLE, MPI_SUM, 0, job->group_comm);

    MPI_Barrier(job->group_comm);
    if(rank == 0){
        divide(job->job_results->avg_times, (double)group_sz, TIME_ARR_SZ);
        divide(job->job_results->variances, (double)group_sz, TIME_ARR_SZ);
        report_job_stats(job);
    }
    MPI_Barrier(job->group_comm);
}

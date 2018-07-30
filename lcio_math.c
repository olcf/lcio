//
// Created by Matthew Bachstein on 6/6/18.
//

#include "lcio.h"

static void seed_rng(long seed){
    srand48(seed);
}

double gen_rand_normal(double mean, double stddev) {
    static double n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached) {
        double x, y, r;
        do
        {
            x = 2.0*drand48()/RAND_MAX - 1;
            y = 2.0*drand48()/RAND_MAX - 1;

            r = x*x + y*y;
        } while (r == 0.0 || r > 1.0);

        {
            double d = sqrt(-2.0*log(r)/r);
            double n1 = x*d;
            n2 = y*d;
            double result = n1*stddev + mean;
            n2_cached = 1;
            return result;
        }
    }
    else
    {
        n2_cached = 0;
        return n2*stddev + mean;
    }
}

int gen_rand_uniform(int max){
    return (int) floor(max * drand48());
}

double gen_random_gamma(double k, double theta){
    // this is modeled after the GNU Sci lib version
    if(k < 1){
        double u = drand48();
        return gen_random_gamma(1.0 + k, theta) * pow(u, 1.0 / k);
    }

    double d,c,x,v,u;

    d = k - 1.0 / 3.0;
    c = (1.0/3.0) / sqrt(d);
    while (1){
        do {
            x = gen_rand_normal(0, 1);
            v = 1.0 + c * x;
        } while(v >= 0.0);

        v = v * v * v;
        u = drand48();
        if (u > 1 - 0.0331 * x * x * x * x)
            break;

        if (log (u) > 0.5 * x * x + d * (1 - v + log (v)))
            break;
    }
    return theta * d * v;
}

void divide(double* arr, double divisor, int len){
    int i;

    for(i=0; i < len; i++){
        arr[i] = arr[i] / divisor;
    }
}

double max(const double* arr, int num_runs){
    double max = -1.0;
    int i;
    for(i=0; i < num_runs; i++){
        max = (arr[i] > max) ? arr[i] : max;
    }
    return max;
}

double min(const double* arr, int num_runs){
    double min = 9999999999999999.0;
    int i;
    for(i=0; i < num_runs; i++){
        min = (arr[i] < min) ? arr[i] : min;
    }
    return min;
}

double avg(const double* arr, int num_runs){
    /*
     * NOTE: double* arr is assumed to be zeroed out
     * before the values are plugged in.
     * See file_test.c:[176-177]
     */
    double accum = 0.0;
    int i;
    for(i=0; i < num_runs; i++){
        accum += arr[i];
    }

    return accum / (double) num_runs;
}

double variance(const double* arr, int num_runs){
    double mean = avg(arr, num_runs);
    int i;

    double* tmp;
    double accum = 0.0;

    tmp = malloc(sizeof(double) * num_runs);

    for (i=0; i < num_runs; i++){
        tmp[i] = pow(fabs(arr[i] - mean), 2);
    }
    for(i=0; i < num_runs; i++){
        accum += tmp[i];
    }
    free(tmp);
    return accum / (double) (num_runs-1);
}

double stddev(const double* arr, int num_runs){
    double var = variance(arr, num_runs);
    return sqrt(var);
}

double calc_bw(double time, lcio_job_t* job){
    // 'time' is the sum of all the individual process times
    // so, divide by the total number of files.
    double rate = (double)job->blk_sz * (double)job->num_files / (time * (1 << 20));
    // this is in MiB/sec
    return isinf(rate) ? 0 : rate;
}

/*
 * These are indexed by the stage of the the operation
 *  0: create
 *  1: write
 *  2: stat
 *  3: read
 *  4: remove
 *  5: tree create
 *  6: tree remove
 *
 *  so. e.g. max_times[0] is the max of the create operations
 */

void process_times(lcio_results_t* res, int num_runs){
    double* tmp_array;
    int i, j;

    tmp_array = malloc(sizeof(double) * num_runs);

    for(j=0; j < TIME_ARR_SZ; j++){
        for(i=0; i < num_runs; i++){
            tmp_array[i] = res->raw_times[i][j];
        }

        res->max_times[j] = max(tmp_array, num_runs);
        res->min_times[j] = min(tmp_array, num_runs);
        res->avg_times[j] = avg(tmp_array, num_runs);
        res->variances[j] = variance(tmp_array,num_runs);
    }

}

void process_bandwidths(lcio_job_t* job){
    int i;

    for( i = 0; i < TIME_ARR_SZ; i++){
        job->job_results->max_bandwidths[i] = calc_bw(job->job_results->max_times[i], job);
        job->job_results->min_bandwidths[i] = calc_bw(job->job_results->min_times[i], job);
        job->job_results->avg_bandwidths[i] = calc_bw(job->job_results->avg_times[i], job);
        job->job_results->var_bandwidths[i] = calc_bw(job->job_results->variances[i], job);
    }
}

void report_job_stats(lcio_job_t* job){
    const char header[]="==========================================================================\n"
                         "%12s    %12s  %12s  %12s  %12s\n";
    const char lines[] ="--------------------------------------------------------------------------\n";
    const char fmt[] =  "%12s ::  %.8lf  %.8lf  %.8lf  %.8lf\n";
    const char fmt2[] = "%12s :: %12.4lf  %12.4lf  %12.4lf  %12.4lf\n";

    int i;
    process_bandwidths(job);

    printf("\nJob: %s with %d processes of type %c\nEngine: %s\n",
           job->type, job->num_pes,job->mode, job->engine);
    printf("Results of %d runs\n\n", job->num_runs);

    printf(header, "", "Max", "Min", "Avg", "Stddev");
    printf(lines);
    for (i = 0; i < TIME_ARR_SZ; i ++) {

        printf(fmt, g_op_indicies[i],
               job->job_results->max_times[i],
               job->job_results->min_times[i],
               job->job_results->avg_times[i],
               sqrt(job->job_results->variances[i]));
    }
    printf(lines);
    for (i = 0; i < TIME_ARR_SZ; i ++) {
        printf(fmt2, g_op_indicies[i],
               job->job_results->max_bandwidths[i],
               job->job_results->min_bandwidths[i],
               job->job_results->avg_bandwidths[i],
               sqrt(job->job_results->var_bandwidths[i]));
        //printf(lines);
    }
    printf(lines);
    printf("\n\n");
    fflush(stdout);
}



off_t convert_suffix(const char *sz){
    off_t base;
    off_t exp;
    char scale;
    int err;

    err = sscanf(sz,"%lld%c", &base, &scale);
    if(err != 2){
        fprintf(stderr, "read: %s\n", sz);
        ELOCAL("Did not convert size suffix");
    }

    switch(scale){
        case 't':
            exp = (1 << 40);
            break;
        case 'g':
            exp = (1 << 30);
            break;
        case 'm':
            exp = (1 << 20);
            break;
        case 'k':
            exp = (1 << 10);
            break;
        default:
            exp = 1;
    }

    return (off_t) base * exp;
}


float* compute_dist(lcio_dist_t* dist){
    int i;
    static int cached = 0;
    float sum = 0.0;
    static float *arr;

    if(cached == 0){
        arr = malloc(sizeof(float) * dist->len);
        memcpy(arr, dist->array , sizeof(float) * dist->len);

        for(i = 0; i < dist->len; i++){
            sum += arr[i];
        }

        for(i=0; i < dist->len; i++){
            arr[i] /= sum;
        }
        cached = 1;
        return arr;
    }
    else {
        return arr;
    }
}


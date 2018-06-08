//
// Created by Matthew Bachstein on 6/6/18.
//

#include "lcio.h"
#include "lcio_math.h"


float gen_rand_normal(float mean, float stddev, long seed) {
    static double n2 = 0.0;
    static int n2_cached = 0;
    srand48(seed);
    if (!n2_cached)
    {
        double x, y, r;
        do
        {
            x = 2.0*drand48()/RAND_MAX - 1;
            y = 2.0*drand48()/RAND_MAX - 1;

            r = x*x + y*y;
        }
        while (r == 0.0 || r > 1.0);

        {
            double d = sqrt(-2.0*log(r)/r);
            double n1 = x*d;
            n2 = y*d;
            double result = n1*stddev + mean;
            n2_cached = 1;
            return (float)result;
        }
    }
    else
    {
        n2_cached = 0;
        return (float)n2*stddev + mean;
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
        res->stddevs[j] = stddev(tmp_array,num_runs);

    }

}

void report_job_stats(lcio_job_t* job){

}
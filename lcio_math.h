//
// Created by Matthew Bachstein on 6/6/18.
//

#ifndef LCIO_LCIO_MATH_H
#define LCIO_LCIO_MATH_H

#include <math.h>
#include <unistd.h>

#define TIME_ARR_SZ 5

static const char* g_op_indicies[TIME_ARR_SZ] = {"create",
                                   "write",
                                    "stat",
                                    "read","remove"};

static const float g_dist_real[26] = {45.00, 13.00, 21.00, 16.00, 11.00,
                               5.00, 25.00, 11.00, 26.00, 22.00,
                               49.00, 45.00, 6.00, 12.00, 18.00,
                               1.00,  0.50,  0.50, 0.50, 0.01,
                               0.00, 0.00, 0.00, 0.00, 0.00, 0.00};

static const float g_dist_test[26] = {45.00, 13.00, 21.00, 16.00, 11.00,
                               5.00, 25.00, 11.00, 26.00, 22.00,
                               49.00, 45.00, 6.00, 0.00, 0.00,
                               0.00,  0.00,  0.00, 0.00,  0.00,
                               0.00, 0.00, 0.00, 0.00, 0.00, 0.00};

static const char *sizes[26] = {"4k","8k", "16k", "32k", "64k",
                         "128k", "256k","512k", "1m", "2m",
                         "4m", "16m", "32m","64m","128m",
                         "256m","512m","1g", "4g", "64g",
                         "128g", "256g", "512g", "1t", "4t", "8t"};


double gen_rand_normal(double mean, double stddev);
double gen_random_gamma(double k, double theta);
int gen_rand_uniform(int max);
static void seed_rng(long);

typedef struct lcio_results {
    double* max_times;
    double* min_times;
    double* avg_times;
    double* variances;
    double* max_bandwidths;
    double* min_bandwidths;
    double* avg_bandwidths;
    double* var_bandwidths;
    double** raw_times;

} lcio_results_t;

void divide(double*, double, int);

off_t convert_suffix(const char *);

void process_times(lcio_results_t*, int);


#endif //LCIO_LCIO_MATH_H


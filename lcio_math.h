//
// Created by Matthew Bachstein on 6/6/18.
//

#ifndef LCIO_LCIO_MATH_H
#define LCIO_LCIO_MATH_H

#include <math.h>
#include "lcio.h"

#define TIME_ARR_SZ 7

static const char* g_op_indicies[TIME_ARR_SZ] = {"create",
                                   "write",
                                    "stat",
                                    "read","remove",
                                    "tree create",
                                    "tree_remove"};

float gen_rand_normal(float mean, float stddev, long seed);

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

void process_times(lcio_results_t*, int);


#endif //LCIO_LCIO_MATH_H


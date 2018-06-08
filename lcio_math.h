//
// Created by Matthew Bachstein on 6/6/18.
//

#ifndef LCIO_LCIO_MATH_H
#define LCIO_LCIO_MATH_H

#include <math.h>
#include "lcio.h"

#define TIME_ARR_SZ 8

float gen_rand_normal(float mean, float stddev, long seed);

typedef struct lcio_results {
    double* max_times;
    double* min_times;
    double* avg_times;
    double* stddevs;
    double** raw_times;

} lcio_results_t;



void process_times(lcio_results_t*, int);


#endif //LCIO_LCIO_MATH_H


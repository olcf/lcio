//
// Created by Matthew Bachstein on 6/6/18.
//

#ifndef LCIO_LCIO_MATH_H
#define LCIO_LCIO_MATH_H

#include <math.h>
float gen_rand_normal(float mean, float stddev, long seed);

typedef struct lcio_results {
double max_time;
double min_time;
double avg_time;
double bandwidth;
} lcio_results_t;
#endif //LCIO_LCIO_MATH_H

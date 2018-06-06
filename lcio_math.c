//
// Created by Matthew Bachstein on 6/6/18.
//

#include "lcio.h"
#include <math.h>

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

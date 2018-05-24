//
// Created by Matthew Bachstein on 5/23/18.
//

#include <time.h>
#include <sys/time.h>

#include "lcio.h"

/*
 * Timing structs for general use
 */

double get_time(void){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (double)(tv.tv_sec * 1000000) + (double)(tv.tv_usec);
}

double elapsed_time(double t1, double t2){
    if (t1 > t2) return t1-t2;
    else return t2-t1;
}




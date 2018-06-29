//
// Created by Matthew Bachstein on 5/23/18.
//

#include <time.h>
#include <sys/time.h>

#include "lcio.h"

/*
 * Timing structs for general use
 * all times in microseconds
 */

double get_time(void){
    return MPI_Wtime();
}

double elapsed_time(double t1, double t2){
    return fabs(t1 -t2);
}




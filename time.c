/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the LCIO project.
 ****************************************************************************/

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




//
// Created by Matthew Bachstein on 5/23/18.
//

#ifndef LCIO_LCIO_H
#define LCIO_LCIO_H

double get_time(void);
double elapsed_time(double, double);

/*
 * lcio_engine_t describes the operations that lcio will perform.
 * These are function pointers that can be initialized with any
 * of the supported IO libraries (POSIX, MPI_IO, AIO) with the
 * goal of creating a uniform interface for the function calls.
 */
typedef struct lcio_engine {
    char *name;
    void*(*create)(void*);
    void*(*open)(void*);
    void*(*close)(void*);
    void*(*delete)(void*);
    void*(*stat)(void*);
    void*(*mkdir)(void*);
    void*(*rmdir)(void*);
    void*(*fsync)(void*);

} lcio_engine_t;


typedef struct

#endif //LCIO_LCIO_H

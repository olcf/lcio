cmake_minimum_required(VERSION 3.10)
project(lcio C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_library(posix MODULE posix.c)

add_executable(lcio
        conf_parser.c
        conf_parser.h
        main.c
        lcio.h
        time.c
        params.c
        file_test.c)

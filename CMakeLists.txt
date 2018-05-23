cmake_minimum_required(VERSION 3.10)
project(lcio C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(lcio
        conf_parser.c
        conf_parser.h
        main.c)

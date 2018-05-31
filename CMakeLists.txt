cmake_minimum_required(VERSION 3.10)
project(lcio C)

set(CMAKE_C_STANDARD 11)

find_package(MPI REQUIRED)
add_definitions(-DOMPI_SKIP_MPICXX)

include_directories(.)
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_library(posix MODULE lib/posix.c)

add_executable(lcio
        conf_parser.c
        conf_parser.h
        main.c
        lcio.h
        time.c
        params.c
        file_test.c)

target_link_libraries(lcio PRIVATE ${MPI_C_LIBRARIES})
target_include_directories(lcio PRIVATE ${MPI_C_INCLUDE_PATH})
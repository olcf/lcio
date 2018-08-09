# LCIO
Lifecycle based file system I/O evaluation tools.  
lcio is designed to synthetically age and test large  
parallel filesystems (Luster, GPFS, Ceph, etc)

## Requirements
Cmake > 3.0  
MPI > 3.0 (tested with Open MPI 1.10 and 3.1, 1.10 has some issues that were fixed in later versions)  
C11 compatible compiler  

---
## Concept

lcio takes a .ini file that specifies how the run is to be structured, and another .ini file 
that defines the size distribution of the files. It supports 
two different 'types' of jobs 'file_tree' and 'rw'. 'file_tree' is the synthetic aging 
procedure, and 'rw' is a simple IOR type benchmark.  
This is to allow for multiple stages in the whole run, stage one being an initial 
benchmark, stage two aging the system for some amount of operations and utilization, then stage 3 runs another
benchmark to see how the performance has changed, then another aging step, etc.   


## Quick start  
The [example config](config_example.ini) file contains some example configurations of both types of runs. 
The [example dist](dist_example.ini) file has an example file size distribution. 

## Job types
A 'size type' looks like \[4k, 128M, 23G, 4t, etc\] (upper or lower case k,m,g,t).  
All 'int' fields should be positive. Putting a negative value is UB. 
### Aging 
Aging is accomplished by the 'file_tree' job type. Required fields are:  
type = "file_tree"  
tmp_dir = directory to write into  
engine = one of "posix" or "mpiio"   
mpi_num_pes = int   
num_files = int, total number of files, split evenly over PEs   
buffer_size = size type, size of buffer used to fill a file (i.e. 512k buffer, 2M file => 4 writes)   
fsync = 0 | 1 (false, true), call 'fsync' after each operation (write, read, etc)    
ops = int, number of aging ops (1 op = delete + write)
overlap = int , number of processes to share a directory  
epochs = int, number of times to do the set of aging ops, barrier sync'd   
clean = 0 | 1, clean 'tmp_dir' after job is finished


### RW  
'rw' is a simple IOR type benchmark measuring time and bandwidth. Required fields
are:  
type = 'rw'  
mode = 'U' | 'S', (unique, shared) process per dir or all processes in same dir   
tmp_dir = directory to write into  
buffer_size = size type, size of buffer used to fill files of size 'block_size' 
engine = 'posix' or 'mpiio'  
mpi_num_pes  = int  
num_files = int, total number of files, split evenly over PEs  
block_size = size type, size of files (Buffer_size MUST evenly divide block_size, best to stick to powers of 2)  
fsync = 0 | 1 (false, true), call 'fsync' after each operation (write, read, etc)   
clean = 0 | 1, clean 'tmp_dir' after job is finished


## Distribution
The distribution ini file has only one section, \[dist\], with entries like  
4k = 100  
32k = 45  
64k = 32  
  
These are relative counts, so the left hand side is a 'size type' and the right hand side is an 'int'  
# lcio
Lifecycle based file system I/O evaluation tools.  
lcio is designed to synthetically age and test large  
parallel filesystems (Luster, GPFS, Ceph, etc)

## Requirements
Cmake > 3.10  
MPI > 3.0 (tested with Open MPI)

C11 compatible compiler  
(This might be able to be relaxed to C99, the only  
C11 feature I use is Typedef redefinition, I think this could be  
restructured)  
---
## Main Idea

lcio takes a .ini file that specifies how the run is to be structured. It supports 
two different 'types' of jobs 'file_tree' and 'rw'. 'file_tree' is the synthetic aging 
procedure, and 'rw' is a simple IOR type benchmark.  
The idea is that there will be multiple stages in the whole run, stage one being an initial 
benchmark, stage two aging the system for some amount of operations, then stage 3 runs another
benchmark to see how the performance has changed, then another aging step, etc.  

## Quick start  
The [test.ini](test.ini) file contains some example configurations of both types of runs.




# estimate-l1-cache

This short program helps in estimating the cache line size and the associativity of the L1 cache in a linux environment. 
It uses the following Intel instructions : RDTSC, RDTSCP, CPUINFO and does an analysis on the data that is generated.

The code is executed in kernel mode to ensure accurate results.

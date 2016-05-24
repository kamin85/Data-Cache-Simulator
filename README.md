# Data-Cache-Simulator
Data cache simulator c++ project for computer organization class


The .cpp file is the actual simulator written in c++ acording to the .pdf project instructions.
The .config file is the file the .cpp reads from to get the configuration of the data cache being represented, can be changes to represent different number of sets (no more than 8192), different set size (no more than 8), and different line size (equal to or less than 8)
The .dat file is the file that the simulator uses to simulate the instructions (read or write) found inside it. In the format R:4:b0, where first is R or W for read or write, then the size of the reference, and then the hexaddress. 

The simulator uses a LRU replacement algorithm meaning that the least recently used for which slot in the set to replace when filled. Also a write-back write-allocate policy is used.

The simulation prints out the cache configuration followed by the results for each reference in the form:
Reference #, Access(R or W), hexaddress, Tag, Index, Offset, Result (hit or miss), # of memory references
Lastly some simulation summary statistics are printed out, these include total hits, misses, and accessess as well as both the hit and miss ratios.

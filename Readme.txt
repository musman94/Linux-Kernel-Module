A Linux kernel module that does the following things:
- Prints the process tree: Prints information about each and every running process currently.
- Prints VM info: Prints virtual memory layout information of the process whose pid is specified at command line while the module is being inserted.
- Prints open files information: Prints open files information about a process whose pid is specified at insertion time.

Sample invocation: insmod mem.ko id=546
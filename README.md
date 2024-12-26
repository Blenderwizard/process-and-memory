# process-and-memory

Drivers and Interrupts is a 42 Linux Kernel Module Project written in C, the goal is to write a new Linux Syscall that allows a user to get information about a specific PID.

This information includes it's state, the address of it's stack, the age of the process, an array of it's children's pids, the pid of the parent, the process's root path and working directory.

Correct usage of the Syscall is detailed below:
```c
struct pid_info data;

// Get size of children pid array
long c = get_pid_info(NULL, pid);
data.children = malloc(sizeof(int) * c);
// Get data about process
long ret = get_pid_info(&data, pid);
/*
    Do Stuff
*/
// Don't forget to free data.children
free(data.children);
```

> **Note**
>
> The Makefile was configured for my custom Linux distribution, It is likely you'd need to make your own for your distribution.
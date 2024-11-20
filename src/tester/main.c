//
// Created by jrathelo on 11/18/24.
//

#include <linux/kernel.h>
#include <linux/limits.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define __NR_get_pid_info 463

struct pid_info {
    int pid;
    unsigned int state;
    const void *stack;
    unsigned long long age;
    int *children;
    int parent_pid;
    char root_path[PATH_MAX];
    char working_directory[PATH_MAX];
};


long get_pid_info(struct pid_info * ret, int pid) {
    return syscall(__NR_get_pid_info, ret, pid);
}

int recurse_get_pid_info(int pid) {
    struct pid_info data;

	long c = get_pid_info(NULL, pid);
    data.children = malloc(sizeof(int) * c);
	long ret = get_pid_info(&data, pid);
    if (ret) {
        return ret;
    }
    if (data.pid == 0) {
        return 1;
    } 

    printf("Pid: %d\n", data.pid);
    printf("# of children: %ld\n", c);
    printf("Children: [");
    for(long i = 0; i < c; i++) {
        printf("%d, ", data.children[i]);
    }
    printf("]\n");
    printf("State; %d\n", data.state);
    printf("Age : %lld ms\n", data.age / 1000000);
    printf("Stack address: %p\n", data.stack);
    printf("Parent PID: %d\n", data.parent_pid);
    printf("Root Path: %s\n", data.root_path);
    printf("Current Working Directory: %s\n\n", data.working_directory);
    int r = 0;
    for(long i = 0; i < c; i++) {
        r += recurse_get_pid_info(data.children[i]);
    }
    return r;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s [pid]\n", argv[0]);
		return -1;
	}
    return recurse_get_pid_info(atoi(argv[1]));
}


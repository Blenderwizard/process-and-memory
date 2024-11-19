//
// Created by jrathelo on 11/18/24.
//

#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define __NR_get_pid_info 463

long get_pid_info_syscall(int pid) {
    return syscall(__NR_get_pid_info, pid);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s [pid]\n", argv[0]);
		return -1;
	}
	return get_pid_info_syscall(atoi(argv[1]));
}


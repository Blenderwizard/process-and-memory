//
// Created by jrathelo on 11/18/24.
//

#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/timekeeping.h>
#include <linux/dcache.h>
#include <linux/rcupdate.h>
#include <linux/path.h>

SYSCALL_DEFINE1(get_pid_info, int, pid) {
    struct task_struct *task;
    char buf[PATH_MAX];
    char *path;
    struct timespec64 current_time;

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task) {
        pr_info("Process ID: %d\n", pid);
        pr_info("Process Name: %s\n", task->comm);
        pr_info("Process State: %x\n", task->__state);
        pr_info("Stack address: %p\n", task->stack);
        ktime_get_boottime_ts64(&current_time);
        u64 age_ms = (timespec64_to_ns(&current_time) - task->start_time) / 1000000;
        pr_info("Process Age: %llu ms\n", age_ms);
        pr_info("Children list address: %llu\n", task->children);
        pr_info("Parent PID: %d\n", task->parent->pid);
        if (!task->fs) {
            pr_info("No filesystem context found\n");
            rcu_read_unlock();
            return 0;
        }
        path = d_path(&task->fs->root, buf, PATH_MAX);
        if (IS_ERR(path)) {
            pr_info("Failed to resolve root path\n");
        } else {
            pr_info("Root Path: %s\n", path);
        }
        path = d_path(&task->fs->pwd, buf, PATH_MAX);
        if (IS_ERR(path)) {
            pr_info("Failed to resolve PWD\n");
        } else {
            pr_info("Current Working Directory: %s\n", path);
        }
    }
    rcu_read_unlock();
    return 0;
}

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

struct pid_info {
    int pid;
    u32 state;
    const void *stack;
    u64 age;
    // int * children;
    int parent_pid;
    char root_path[PATH_MAX];
    char working_directory[PATH_MAX];
};

static long    __sys_get_pid_info(struct pid_info __user * p_info_struct, int pid) {
    struct task_struct *task;
    char * path;
    struct timespec64 current_time;

    struct pid_info *data = (struct pid_info *) kzalloc(sizeof(struct pid_info), GFP_KERNEL);
    if (data == NULL) {
        return -ENOMEM;
    }

    int error = copy_from_user(data, p_info_struct, sizeof(struct pid_info));
    if (error) {
        kfree(data);
        return -EFAULT;
    }

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task) {
        data->pid = pid;
        data->state = task->__state;
        data->stack = task->stack;
        ktime_get_boottime_ts64(&current_time);
        data->age = (timespec64_to_ns(&current_time) - task->start_time);
        if (task->parent) {
            data->parent_pid = task->parent->pid;
        } else {
            data->parent_pid = 0;
        }
        if (!task->fs) {
            memset(data->root_path, 0, PATH_MAX);
            memset(data->working_directory, 0, PATH_MAX);
            rcu_read_unlock();
            return 0;
        }
        path = d_path(&task->fs->root, data->root_path, PATH_MAX);
        if (IS_ERR(path)) {
            memset(data->root_path, 0, PATH_MAX);
        } else {
            memcpy(data->root_path, path, PATH_MAX);
        }
        path = d_path(&task->fs->pwd, data->working_directory, PATH_MAX);
        if (IS_ERR(path)) {
            memset(data->working_directory, 0, PATH_MAX);
        } else {
            memcpy(data->working_directory, path, PATH_MAX);
        }
    }
    rcu_read_unlock();
    error = copy_to_user(p_info_struct, data, sizeof(struct pid_info));
    if (error) {
        kfree(data);
        return -EFAULT;
    }
    kfree(data);
    return 0;
}

SYSCALL_DEFINE2(get_pid_info, struct pid_info __user *, p_info_struct, int, pid) {
    return __sys_get_pid_info(p_info_struct, pid);
}

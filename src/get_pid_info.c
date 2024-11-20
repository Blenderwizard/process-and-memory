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
    unsigned int state;
    const void *stack;
    unsigned long long age;
    int *children;
    int parent_pid;
    char root_path[PATH_MAX];
    char working_directory[PATH_MAX];
};

static long    __sys_get_pid_info(struct pid_info __user * p_info_struct, int pid) {
    struct task_struct *task;
    char * path;
    struct timespec64 current_time;
    size_t size = 0;
    int *child_list = NULL;

    if (p_info_struct == NULL) {
        rcu_read_lock();
        task = pid_task(find_vpid(pid), PIDTYPE_PID);
        if (task) {
            size = list_count_nodes(&(task->children));
            rcu_read_unlock();
            return size;
        }
        rcu_read_unlock();
        return 0;
    }

    struct pid_info *data = (struct pid_info *) kzalloc(sizeof(struct pid_info), GFP_KERNEL);
    if (data == NULL) {
        return -ENOMEM;
    }
    memset(data, 0, sizeof(struct pid_info));

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
        size = list_count_nodes(&(task->children));
        if (size == 0) {
            child_list = NULL;
        } else {
            child_list = kzalloc(sizeof(int) * size, GFP_KERNEL);
            if (child_list == NULL){
                kfree(data);
                rcu_read_unlock();
                return -ENOMEM;
            }
            struct task_struct *child_task;
            size_t index = 0;
            list_for_each_entry(child_task, &task->children, sibling) {
                child_list[index] = child_task->pid;

                pr_info("%d\n", child_list[index]);
                index ++;
            }
        }
    }
    rcu_read_unlock();
    if (child_list != NULL) {
        pr_info("HERE: %d\n", child_list[0]);
        error = copy_to_user(data->children, child_list, sizeof(int) * size);
        if (error) {
            pr_info("HERE?\n");
            kfree(child_list);
            kfree(data);
            return -EFAULT;
        }
        kfree(child_list);
    }
    pr_info("HERE6\n");
    error = copy_to_user(p_info_struct, data, sizeof(struct pid_info));
    if (error) {
        kfree(data);
        return -EFAULT;
    }
    kfree(data);
    pr_info("HERE7\n");
    return 0;
}

SYSCALL_DEFINE2(get_pid_info, struct pid_info __user *, p_info_struct, int, pid) {
    return __sys_get_pid_info(p_info_struct, pid);
}

#include "os.h"

#include "syscalls.h"

os_errorcode os_get_task_info(OSTaskInfo* list, uint32_t list_max_size, uint32_t* num_tasks) {
    return syscall_get_task_info(list, list_max_size, num_tasks);
}

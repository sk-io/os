#include "shell.h"

#include "console.h"
#include "util.h"
#include "physalloc.h"
#include "log.h"
#include "kmalloc.h"
#include "memory.h"
#include "tasks.h"
#include "syscall.h"
#include "ramdisk.h"

void shell_execute(const char* code) {
    if (strcmp(code, "\n") == 0)
        return;

    if (strncmp(code, "ls", 2) == 0) {
        FRESULT res;
        DIR dir;
        res = f_opendir(&dir, "");
        assert_msg(res == FR_OK, "f_opendir");

        FILINFO info;
        while (true) {
            res = f_readdir(&dir, &info);
            assert_msg(res == FR_OK, "f_readdir");

            if (info.fname[0] == 0)
                break;
            
            kernel_log("%s", info.fname);
        }

        f_closedir(&dir);
        return;
    }

    if (strncmp(code, "run", 3) == 0) {
        const char* path = code + 4;
        create_user_task(path, NULL);
        return;
    }

    if (strncmp(code, "shmem", 5) == 0) {
        kernel_log("[kernel shmem]");
        shmem_print_mappings(NULL);
        for (int i = 0; i < MAX_TASKS; i++) {
            Task* task = &tasks[i];
            if (task->state == TASK_STATE_DEAD)
                continue;
            
            kernel_log("[task %u shmem]", task->id);
            shmem_print_mappings(&task->shmem);
        }
        return;
    }

    if (code[0] == 'p') {
        kernel_log("physical pageframes used: %u", pmm_get_total_allocated_pages());
        return;
    }

    if (code[0] == 'v') {
        kernel_log("mapped virtual pages: %u", mem_num_vpages);
        return;
    }

    if (code[0] == 'h') {
        kernel_log("kernel heap use: %u bytes", kmalloc_get_total_bytes());
        return;
    }

    if (code[0] == 'a') {
        void* test = kmalloc(666);
        kernel_log("%p", test);
        return;
    }

    if (code[0] == 'k') {
        u32 num = (u32) code[1] - (u32) '0';
        kill_task(1000 + num);
        return;
    }

    if (code[0] == 't') {
        kernel_log("num tasks: %u", num_tasks);
        return;
    }

    kernel_log("unknown command");
}

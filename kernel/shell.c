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
        create_user_task(path);
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

    if (code[0] == 'd') {
        kernel_log("current pagedir: %x", mem_get_current_page_directory());
        return;
    }

    console_print(code);
}

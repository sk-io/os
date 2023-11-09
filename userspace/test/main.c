#include <os.h>
#include <types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../../kernel/syscall_list.h"

#define width 300
#define height 240

void crash() {
    int* frick = (int*) 0x112345;
    *frick = 1234;
}

int main(int argc, char* argv[]) {
    // printf("hello!!");

    // for (int i = 0; i < 1; i++) {
    int shmem = os_create_shared_mem(50000);
    os_destroy_shared_mem(shmem);
    // }

    // asm volatile(
    //     "int $0x80"
    //     :: "a"(SYSCALL_DEBUG)
    // );

    // int heap_start = os_get_heap_start();
    // int heap_end = os_get_heap_end();
    // os_printf("heap_start=%x heap_end=%x", heap_start, heap_end);

    // os_printf("allocating...");
    // int* test[100];
    // for (int i = 0; i < 100; i++)
    //     test[i] = malloc(4);
    
    // os_printf("writing...");
    // for (int i = 0; i < 100; i++)
    //     *(test[i]) = i;

    // os_printf("freeing...");
    // for (int i = 0; i < 100; i++)
    //     free(test[i]);
    
    // crash();

    // *(u32*) 0x123455 = 1234; // this causes clang with -O2 to crap out completely for some reason

    // char* test = "hello!";
    // char buf[64];
    // memset(buf, 0, 64);
    // strcpy(buf, test);
    // os_printf("test: %s", buf);
    return 0;
}

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

void write_test() {
    const char* text = "It's working!";

    FILE* file = fopen("out.txt", "w");
    fwrite(text, 1, strlen(text) + 1, file);
    fclose(file);
}

int main(int argc, char* argv[]) {
    write_test();

    FILE* file = fopen("out.txt", "r");

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("file size: %u\n", size);

    char buf[16];
    fread(buf, 1, sizeof(buf), file);

    os_print(buf);

    fclose(file);

    // for (int i = 0; i < 1; i++) {
    // int shmem = os_create_shared_mem(1000);

    // u32* data = os_map_shared_mem(shmem);
    // *data = 235;
    // os_unmap_shared_mem(shmem);

    // os_destroy_shared_mem(shmem);
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

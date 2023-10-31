#include <os.h>
#include <types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define width 300
#define height 240

int main(int argc, char* argv[]) {
    // printf("hello!!");

    int heap_start = os_get_heap_start();
    int heap_end = os_get_heap_end();
    os_printf("heap_start=%x heap_end=%x", heap_start, heap_end);

    os_printf("allocating...");
    int* test[100];
    for (int i = 0; i < 100; i++)
        test[i] = malloc(4);
    
    os_printf("writing...");
    for (int i = 0; i < 100; i++)
        *(test[i]) = i;

    // char* test = "hello!";
    // char buf[64];
    // memset(buf, 0, 64);
    // strcpy(buf, test);
    // os_printf("test: %s", buf);
    return 0;
}

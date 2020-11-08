#include <stdio.h>
#include "memlib.h"

int main(int argc, const char** argv) {
    printf("Hello World\n");
    mem_init();
    char* heap = (char*)mem_sbrk(10);
    return 0;
}

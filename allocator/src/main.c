#include <stdio.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

void malloc_free() {
    const char* msg = "Hello";
    
    char* buf = (char*)mm_malloc(9);

    strcpy(buf, msg);
    printf("memory content: %s\n", buf);
    
    mm_free(buf);
}

int main(int argc, const char** argv) {
    mem_init();
    mm_init();

    malloc_free();
    
    mem_free();
    return 0;
}

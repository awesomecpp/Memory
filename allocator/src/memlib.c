#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"

#define MAX_HEAP (20*(1<<20))  /* 20 MB */

static char* mem_heap;
static char* mem_brk;
static char* mem_max_addr;

/*Initialize the memory model*/
void mem_init(void) {
    mem_heap = (char*)malloc(MAX_HEAP); /*heap start address*/
    mem_brk = (char*)mem_heap; /*current heap top*/
    mem_max_addr =  (char*)(mem_heap + MAX_HEAP);/*max legal heap address plus 1*/
}

/*Extend the heap by incr bytes*/
void* mem_sbrk(int incr) {
    char* old_brk = mem_brk;

    if((incr <0) || (mem_brk + incr) > mem_max_addr) {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
        return (void*)-1;
    }

    mem_brk += incr;
    return old_brk;
}

void mem_free(void) {
    free(mem_heap);
    mem_heap = NULL;
    mem_brk = NULL;
    mem_max_addr = NULL;
}

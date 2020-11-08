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

void mem_init(void) {

}

void* mem_sbrk(int incr) {

}

#include <unistd.h>

void mem_init(void);
void mem_free(void);
void* mem_sbrk(int incr);

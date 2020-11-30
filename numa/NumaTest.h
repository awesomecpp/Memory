#include <numa.h>
#include <unistd.h>
#include <cstdlib>
#include <sched.h>
#include <time.h>
#include <iostream>

void set_cpu(int cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

inline uint64_t rdtsc()
{
   uint32_t hi, lo;
   asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
   return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

void test_numa_alloc() {
    size_t buf_size = 4096*1024;
    uint64_t num_reads = 0xFFFFFF;
    uint64_t val, t0, cycle_local, cycle_remote;

    set_cpu(0);
    srand(time(0));
    val = 0;

    char* buf_local = (char*)numa_alloc_local(buf_size);
    t0 = rdtsc();
    for(uint64_t i = 0; i <num_reads; ++i) {
        val += buf_local[rand()%buf_size];
        buf_local[rand()%buf_size] = i&0xFF;
    }
    cycle_local = rdtsc() - t0;

    char* buf_remote = (char*)numa_alloc_onnode(buf_size, 0);// only have 1 node
    t0 = rdtsc();
    for(uint64_t i = 0; i <num_reads; ++i) {
        val += buf_remote[rand()%buf_size];
        buf_remote[rand()%buf_size] = i&0xFF;
    }
    cycle_remote = rdtsc() - t0;

    std::cout<<cycle_local/num_reads << " " <<cycle_remote/num_reads <<std::endl;

    numa_free(buf_local, buf_size);
    numa_free(buf_remote, buf_size);
}
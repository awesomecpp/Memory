#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

static const char* options  = "hvs:E:b:t:";
static const char* usage    = "./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>";
static int verbose          = 0;
static int index_bits       = 0;
static int associativity    = 0;
static int block_bits       = 0;
static char* tracefile      = NULL;

//Cache Line
typedef struct {
    long tag;
    long timestamp;
    int valid;
} Block; 

typedef Block* Set;
typedef Set* Cache; 

static Cache cache = NULL;
static long hit = 0;
static long miss = 0;
static long eviction = 0;
static long timer = 0;

void init_cache()
{
    int blk_num = associativity;
    int set_num = 1 << index_bits;
    cache = (Cache)malloc(set_num * sizeof(Set));
    int i, j;
    for(i = 0; i < set_num; ++i) {
        cache[i] = (Set)malloc(blk_num * sizeof(Block));
        for(j = 0; j < blk_num; ++j) {
            cache[i][j].tag = 0;
            cache[i][j].timestamp = 0;
            cache[i][j].valid = 0;
        }
    }
}

void free_cache()
{
    int set_num = 1 << index_bits;

    int i;
    for(i = 0; i < set_num; ++i) {
        free(cache[i]);
    }
    free(cache);
    cache = NULL;
}

int parse_options(int argc, char* const argv[])
{
    char opt;
    while((opt = getopt(argc, argv, options)) != -1) {
        switch(opt) {
            case 'h': printf("%s\n", usage);        break;
            case 'v': verbose = 1;                  break;
            case 's': index_bits = atoi(optarg);    break;
            case 'E': associativity = atoi(optarg); break;
            case 'b': block_bits = atoi(optarg);    break;
            case 't': tracefile = optarg;           break;
            default:
                printf("unknown option, usage: %s\n", usage);
                return 0;
        }
    }
    if(!index_bits || !associativity || !block_bits || !tracefile) {
        printf("invalid option, usage: %s\n", usage);
        return 0;
    }

    return 1;
}

void load_or_store(unsigned long long address)
{
    // unsigned long long blk_offset = address & ((1<<block_bits) -1);
    unsigned long long set_index =  (address >> block_bits) & ((1<<index_bits) - 1);
    long tag = address >> (block_bits + index_bits);

    Set set = cache[set_index];
    int i;
    int target = -1;
    for(i = 0; i < associativity; ++i) {
        if(set[i].tag == tag && set[i].valid) {
            target = i;
            break;
        }
    }
    if(target != -1) {
        hit += 1;
        set[target].timestamp = timer;
        return;
    }

    miss += 1;

    long min_timestamp = -1;
    for(i = 0; i < associativity; ++i) {
        if(set[i].timestamp < min_timestamp || min_timestamp == -1) {
            target = i;
            min_timestamp = set[i].timestamp;
        }
    }

    if(set[target].valid) eviction += 1;
    set[target].tag = tag;
    set[target].timestamp = timer;
    set[target].valid = 1;
}

void trace() 
{
    char operation;
    unsigned long long address;
    int size;
    FILE* fd = fopen(tracefile, "r");
    while(fscanf(fd, "%c %llx,%d", &operation, &address, &size) > 0) {
        timer++;
        switch (operation) {
            case 'I': continue; break;
            case 'L': load_or_store(address); break;
            case 'S': load_or_store(address); break;
            case 'M': load_or_store(address); load_or_store(address); break;
        }
    }
    fclose(fd);
}

int main(int argc, char* const argv[])
{
    if(!parse_options(argc, argv)) return -1;
    init_cache();
    trace();
    free_cache();
    printSummary(hit, miss, eviction);
    return 0;
}

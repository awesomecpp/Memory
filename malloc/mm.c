/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "myteam",
    /* First member's full name */
    "robin",
    /* First member's email address */
    "robin@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//word size
#define WSIZE 4
#define DSIZE 8

#define CHUNKSIZE (1<<12) //4096 bytes

//put or get value from specified address
#define PUT(p, val) (*(unsigned int*)(p) = (val))
#define GET(p) (*(unsigned int*)(p))

//pack header/footer
#define PACK(size, alloc) ((size) | (alloc))

//get size or alloc from header or footer
#define GET_SIZE(p) (GET(p)& ~0x7)
#define GET_ALLO(p) (GET(p)& 0x1)

//get header/footer from block pointer
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

//get prev/next block pointer from current block
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))

//get predecessor and successor store address
#define SUCC(bp) ((char*)(bp))
#define PRED(bp) ((char*)(bp) + WSIZE)

// get pred/succ free block address
#define SUCC_BLKP(bp) (GET(SUCC(bp)))
#define PRED_BLKP(bp) (GET(PRED(bp)))

#define MAX(x, y) ((x)>(y)?(x):(y))

static char* heap_listp;
static char* listp;

static int Index(size_t size);
static void* LIFO(void* bp, void* root);
static void* AddressOrder(void* bp, void* root);
static void* add_block(void* bp);
static void delete_block(void* bp);
static void* imme_coalease(void* bp);
static void* extend_heap(size_t words);
static void* first_fit(size_t asize);
static void* best_fit(size_t asize);
static void place(void* bp, size_t asize);

static void info(void* bp)
{
    printf("BLKP: %p = %u\n", bp, GET(bp));
    printf("HDRP: %p = %u\n", HDRP(bp), GET(HDRP(bp)));
    printf("FTRP: %p = %u\n", FTRP(bp), GET(HDRP(bp)));
    printf("SIZE: %u\n", GET_SIZE(HDRP(bp)));
    printf("ALLO: %u\n", GET_ALLO(HDRP(bp)));
    printf("SUCC: %p = %u\n", SUCC(bp), SUCC_BLKP(bp));
    printf("PRED: %p = %u\n", PRED(bp), PRED_BLKP(bp));
}

static void info_all(void* bp)
{
    printf("PREV BLK\n");
    info(PREV_BLKP(bp));
    printf("CURR BLK\n");
    info(bp);
    printf("NEXT BLK\n");
    info(NEXT_BLKP(bp));
    printf("\n");
}




/* 
 * mm_init - initialize the malloc package.
 * - set the root node address of each segregate list: {16-31}, {32-63}, {64-127}, {128-255}, {256-511}, {512-1023}, {1024-2047}, {2048-4095}, {4096-inf}
 * - set the prologue block and epilogue block
 */
int mm_init(void)
{
    // allocate 9 segregated list root and 3 word for prologue and epilogue
    if( (heap_listp = mem_sbrk(12*WSIZE)) == (void*)-1 )    
        return -1;
    PUT(heap_listp + 0*WSIZE, NULL); //root node address {16-31}
    PUT(heap_listp + 1*WSIZE, NULL); //root node address {32-63}
    PUT(heap_listp + 2*WSIZE, NULL); //root node address {64-127}
    PUT(heap_listp + 3*WSIZE, NULL); //root node address {128-255}
    PUT(heap_listp + 4*WSIZE, NULL); //root node address {256-511}
    PUT(heap_listp + 5*WSIZE, NULL); //root node address {512-1023}
    PUT(heap_listp + 6*WSIZE, NULL); //root node address {1024-2047}
    PUT(heap_listp + 7*WSIZE, NULL); //root node address {2048-4095}
    PUT(heap_listp + 8*WSIZE, NULL); //root node address {4096-inf}

    PUT(heap_listp + 9*WSIZE, PACK(DSIZE, 1));// prologue header
    PUT(heap_listp + 10*WSIZE, PACK(DSIZE, 1));// prologue footer
    PUT(heap_listp + 11*WSIZE, PACK(0, 1)); // epilogue

    listp = heap_listp;
    heap_listp += 10*WSIZE;

    //extend initial free block
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    void* bp;

    if(size == 0) return NULL;

    asize = (size <= DSIZE) ? 2*DSIZE : (size + DSIZE +(DSIZE-1))/DSIZE * DSIZE;

    //first fit
    if((bp = first_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }


    //best fit
    // if((bp = best_fit(asize)) != NULL) {
    //     place(bp, asize);
    //     return bp;
    // }

    if((bp = extend_heap(MAX(CHUNKSIZE, asize)/WSIZE)) == NULL)
        return NULL;

    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 * - coalease it and add to appropriate list
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    ptr = imme_coalease(ptr);
    add_block(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void* extend_heap(size_t words)
{
    size_t size;
    char* bp;

    size = (words%2) ? (words+1)*WSIZE : words*WSIZE; // adjust size to be double word aligned
    if((bp = mem_sbrk(size)) == (void*)-1) return NULL;
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue

    PUT(SUCC(bp), NULL);
    PUT(PRED(bp), NULL);

    //immediate coalease
    bp = imme_coalease(bp);

    //add block to appropiate segregated list
    bp = add_block(bp);

    return bp;
}
/*
* coalese a block with its previous block and next block, and remove them from their original list
*/
static void* imme_coalease(void* bp)
{
    size_t prev_alloc = GET_ALLO(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLO(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) {
        return bp;
    }
    else if(prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_block(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc) {
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        delete_block(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(FTRP(PREV_BLKP(bp)));
        delete_block(NEXT_BLKP(bp));
        delete_block(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}


/// remove a block from a free list
/// - if pred block is not NULL, then change the succ node of pred block to current block's succ node
/// - if succ block is not NULL, then change the pred node of succ block to current block's pred node
static void delete_block(void* bp)
{
    if(PRED_BLKP(bp) != NULL)
        PUT(SUCC(PRED_BLKP(bp)), SUCC_BLKP(bp));
    if(SUCC_BLKP(bp) != NULL)
        PUT(PRED(SUCC_BLKP(bp)), PRED_BLKP(bp));
}

/// add a block to appropriate free list according to its size
static void* add_block(void* bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int index = Index(size);
    void* root = listp + index*WSIZE;

    // two policy for inserting block into free list
    ///LIFO
    return LIFO(bp, root);
    ///Address Order: 
    ///return AddressOrder(bp, root);
}

/// get the index of list root according to block size
static int Index(size_t size)
{
    int idx = 0;
    if(size >= 4096) return 8; // index of {4096, inf}

    size = size >> 5; // the smallest size is 16
    while(size) {
        size = size >> 1;
        idx++;
    }

    return idx;
}

/// LIFO: insert the block as the head of current list
/// Caution: current list is empty
static void* LIFO(void* bp, void* root)
{
    if(SUCC_BLKP(root) != NULL) {
        PUT(PRED(SUCC_BLKP(root)), bp);
        PUT(SUCC(bp), SUCC_BLKP(root));
    }
    else {
        PUT(SUCC(bp), NULL);
    }

    PUT(SUCC(root), bp);
    PUT(PRED(bp), root);
    return bp;
}

/// AddressOrder: Keep an ascending order of free block, which provides better memory utilization in first fit strategy
static void* AddressOrder(void* bp, void* root)
{
    // find the first block whose succ node is larger than bp
    void *succ = root;
    while(SUCC_BLKP(succ) != NULL) {
        succ = SUCC_BLKP(succ);
        if (succ >= bp) break;
    }

    if(succ == root) {
        return LIFO(bp, root);
    }
    else if(SUCC_BLKP(succ) == NULL) {
        PUT(SUCC(succ), bp);
        PUT(PRED(bp), succ);
        PUT(SUCC(bp), NULL);
    }
    else {
        PUT(SUCC(PRED_BLKP(succ)), bp);
        PUT(PRED(bp), PRED_BLKP(succ));
        PUT(SUCC(bp), succ);
        PUT(PRED(succ), bp);
    }

    return bp;
}

/// first fit: try to find a fit in the lists that can accommodate current block
static void* first_fit(size_t asize)
{
    int idx = Index(asize);
    void* succ;
    while(idx <= 8) {
        succ = listp + idx*WSIZE;
        while((succ = SUCC_BLKP(succ)) != NULL) {
            if(GET_SIZE(HDRP(succ)) >= asize && !GET_ALLO(HDRP(succ))) 
                return succ;
        }
        idx++;
    }

    return NULL;
}

static void* best_fit(size_t asize)
{
    int idx = Index(asize);
    void* best = NULL;
    int min_size = 0, size;
    void* succ;
    while(idx <= 8) {
        succ = listp + idx*WSIZE;
        while((succ = SUCC_BLKP(succ)) != NULL) {
            size = GET_SIZE(HDRP(succ));
            if(size >= asize && !GET_ALLO(HDRP(succ)) && (size < min_size || min_size==0)) {
                best = succ;
                min_size = size;
            }
        }
        if(best != NULL) return best;

        idx += 1;
    }

    return NULL;
}

/// allocated into current block
/// - remove current block from its list
/// - split current block
/// - add remaining part into appropriate list
static void place(void* bp, size_t asize) 
{
    size_t rsize;
    rsize = GET_SIZE(HDRP(bp)) - asize;
    delete_block(bp);
    if(rsize >= 2*DSIZE) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(rsize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(rsize, 0));
        add_block(NEXT_BLKP(bp));
    }
    else {
        PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
        PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
    }
}

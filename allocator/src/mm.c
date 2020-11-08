#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memlib.h"
#include "mm.h"

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ((x)>(y) ? (x) : (y))

/* pack size and allocated bit into one word*/
#define PACK(size, alloc) ((size) | (alloc))

/* read/write a word at address p*/
#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

/*get size or allocated bit from header or footer*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* get header/footer pointer from block pointer */
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* get next/prev block from block pointer */
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))

static char* heap_listp = NULL;

static void* extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void* find_fit(size_t asize);
static void* coalesce(void* bp);

/*set prologue and epilogue, and initial chunk*/
int mm_init(void) {
    heap_listp = (char*)mem_sbrk(4 * WSIZE);   

    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 2*WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 3*WSIZE, PACK(0, 1));
    heap_listp += 2*WSIZE;

    /* allocate initial chunk */
    extend_heap(CHUNKSIZE/WSIZE);

    return 0;
}

/*
 * malloc
 * 0. adjust the size to be double word aligned
 * 1. try to find a fit in free list
 * 2. if no fit, try to extend_heap, and split the return chunk
 * */
void* mm_malloc(size_t size) {
    size_t asize; 
    size_t extendsize;
    char* bp;

    if(size == 0) return NULL;

    /*minimum size*/
    if(size <= DSIZE) 
        asize = 2*DSIZE; // plus header size and footer size
    else 
        asize = (size + DSIZE - 1)/DSIZE * DSIZE + DSIZE;

    if((bp = find_fit(asize)) != NULL) {
        place(bp, asize);

        printf("allocate %lu bytes memory at address %p ...\n", asize, bp);
        return bp;
    }

    /*no fit, get more memory and place the block*/
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);

    printf("allocate %lu bytes memory at address %p ...\n", asize, bp);
    return bp;
}

/*
 * free
 */
void mm_free(void* bp) {
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    printf("free %lu bytes memory at address %p ...\n", size, bp);

    coalesce(bp);
}

/* 
 * extend double word aligned chunk 
 * set header and footer
 * coalesce ajacent free block
 */
static void* extend_heap(size_t words) {
    char* bp;
    size_t asize;

    asize = WSIZE * (words % 2 ? words +1 : words);
    
    bp = (char*)mem_sbrk(asize);
    PUT(HDRP(bp), PACK(asize, 0));
    PUT(FTRP(bp), PACK(asize, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

/*
 * boundary tag coalesce
 * case 1: prev alloc, next alloc
 * case 2: prev alloc, next free
 * case 3: prev free, next alloc
 * case 4: prev free, next free
*/
static void* coalesce(void* bp) {
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
   
    /*case 1*/
    if(prev_alloc && next_alloc) {
        return bp; 
    }
    else if(prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); 
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else{
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}

/*find first fit*/
static void* find_fit(size_t size) {
    char* bp;
    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if(size <= GET_SIZE(HDRP(bp)) && !GET_ALLOC(HDRP(bp)))
            return bp;
    }
    return NULL;
}

static void place(void* bp, size_t size) {
    size_t remaining = GET_SIZE(HDRP(bp)) - size;
    if(remaining >= 2*DSIZE) {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remaining, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remaining, 0));
    }
    else {
        PUT(HDRP(bp), PACK(remaining + size, 1));
        PUT(FTRP(bp), PACK(remaining + size, 1));
    }
}

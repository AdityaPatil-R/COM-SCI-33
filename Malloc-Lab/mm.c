/*
 * mm.c - Structure of free/allocated blocks:
 *        -----------------------------------
 *                                                           63        32 31         1   0
 *                                                           |          | |          |   |
 *                                                           -------------------------------      <--------  Header/Footer (header_t/footer_t)
 *                                                          |   Unused   | block_size | a/f |
 *                                                           -------------------------------
 * 
 * 
 *         255                  128 127                   64 63        32 31         1   0
 *         |                      | |                      | |          | |          |   |
 *         ---------------------------------------------------------------------------------      <--------  Free block (block_t)
 *        |          prev          |          next          |   unused   | block_size | a/f |
 *         --------------------------------------------------------------------------------- 
 * 
 * 
 *         255                              96 95         64 63        32 31         1   0
 *         |                                 | |           | |          | |          |   |
 *         ---------------------------------------------------------------------------------      <--------  Allocated block (block_t)
 *        |               unused              |   payload   |   unused   | block_size | a/f |
 *         ---------------------------------------------------------------------------------
 *        
 *        - Header at the start of the block:
 *          - 31 bits: The size of the entire block (Header and footer included)
 *          - 1 bit: 1 - Allocated (a), 0 - Free (f)
 *          
 *        - Footer at the end of the block:
 *          - Same format as the header
 * 
 *        - Free blocks:
 *          - Contains pointers to previous and next blocks in the free list
 *          - Doubly linked list, allows for better coalescing and finding of free blocks
 * 
 *        - Allocated blocks:
 *          - Contains a pointer to the payload
 *          - Is in a union with pointers to previous and next because blocks can only be one of the two types
 *
 *      - Organization of the free list:
 *        ------------------------------
 *         begin                                     end
 *         list                                     list
 *         ---------------------------------------------
 *        | hdr(8:a) | zero or more usr blks | hdr(0:a) |
 *         ---------------------------------------------
 *        | prologue |                       | epilogue |
 *        | block    |                       |    block |
 * 
 *        - Implemented as segregated free lists, as suggested by TA
 *          - Each segregated free list is of the form above
 *          - Better than implicit free lists in terms of space utilization and throughput
 * 
 *        - Organized as a pointer to pointers that head the doubly linked lists for each segregated free list
 *          - First 10 cover a specific range of block sizes between consecutive powers of 2
 *          - The last list contains all blocks that don't fit into the first 10
 *        
 *      - How the allocator manipulates the free list:
 *        --------------------------------------------
 *        - Insertions:
 *          - Freed blocks are added to their corresponding segregated free list based on their size
 * 
 *          - The insertBlock function inserts the block at the beginning of the list
 *            - Growing from the start eliminates the need to traverse the list to insert at the end (O(1) time complexity)
 *        
 *        - Removals: 
 *          - removeBlock function acts as a standard doubly linked list removal function
 *            - Handles cases where the block is the only one in the list, the first block, or the last block
 *        
 *        - Coalescing: 
 *          - Freed blocks attempt to coalesce with adjacent free blocks 
 *          - coalesce adjusts the free list pointers to reflect newly coalesced blocks
 *        
 *        - Fit Finding: 
 *          - find_fit searches through the segregated free lists to find a block that fits the requested size 
 *          - If no such block is found, the heap is extended
 */

#include "memlib.h"
#include "mm.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Your info */
team_t team = {
    /* First and last name */
    "Aditya Patil",
    /* UID */
    "XXXXXXXXX",
    /* Custom message (16 chars) */
    "Pls give me an A",
};

/* Header */
typedef struct
{
    uint32_t allocated : 1;
    uint32_t block_size : 31;
    uint32_t _;
} header_t;

/* Footer */
typedef header_t footer_t;

/* Block */
typedef struct block_t
{
    uint32_t allocated : 1;
    uint32_t block_size : 31;
    uint32_t _;

    union
    {
        struct
        {
            struct block_t* next;
            struct block_t* prev;
        };

        int payload[0];
    } body;
} block_t;

/* This enum can be used to set the allocated bit in the block */
enum block_state
{
    FREE,
    ALLOC
};

/* Constants' definitions */
#define CHUNK_SIZE (1 << 16) /* Initial heap size (bytes) */
#define OVERHEAD (sizeof(header_t) + sizeof(footer_t)) /* Overhead of the header and footer of an allocated block */
#define MIN_BLOCK_SIZE (32) /* The minimum block size needed to keep in a freelist (header + footer + next pointer + prev pointer) */
#define NUM_SEGREGATED_FREE_LISTS (11) /* 11 is the highest number without segmentation faults, and more free lists yields a better throughput */
#define SIZE_COMPARE_THRESHOLD (100) /* Meticulous testing of values between 64 and 128 showed that a SIZE_COMPARE_THRESHOLD of 100 yields the best space utilization (Main improvement seen on binary-bal.rep) */

/* Global variables */
static block_t* prologue; /* Pointer to first block */
static block_t** segregatedFreeLists; /* Pointers to pointers to the first block in each segragated free list (No array as per spec) */

/* Function prototypes for internal helper routines */
static block_t* extend_heap(size_t words); 
static void place(block_t* block, size_t alignSize);
static block_t* find_fit(size_t alignSize);
static block_t* coalesce(block_t* block);
static footer_t* get_footer(block_t* block);
static void printblock(block_t* block);
static void checkblock(block_t* block);
static int indexOfSegregatedFreeListToInsert(int blockSize);
static void insertBlock(block_t* block, int freeListNum);
static void removeBlock(block_t* block, int freeListNum);
static int mm_check(void);

/*
 * mm_init - Initialize the memory manager
 */
/* $begin mm_init */
int mm_init(void)
{
    /* Allocate space for pointers to segregated free lists */
    segregatedFreeLists = mem_sbrk(NUM_SEGREGATED_FREE_LISTS * sizeof(block_t*));

    /* Initialize all segregated free lists with null pointers*/
    for (int i = 0; i < NUM_SEGREGATED_FREE_LISTS; i++)
    {
        segregatedFreeLists[i] = NULL;
    }

    /* Create the initial empty heap */
    if ((prologue = mem_sbrk(CHUNK_SIZE)) == (void*) - 1)
    {
        return -1;
    }

    /* Initialize the prologue */
    prologue->allocated = ALLOC;
    prologue->block_size = sizeof(header_t);

    /* Initialize the first free block */
    block_t* initialBlock = (void*) prologue + sizeof(header_t);
    initialBlock->allocated = FREE;
    initialBlock->block_size = CHUNK_SIZE - OVERHEAD;

    footer_t* init_footer = get_footer(initialBlock);
    init_footer->allocated = FREE;
    init_footer->block_size = initialBlock->block_size;

    /* Updating the new block pointers */
    initialBlock->body.prev = NULL;
    initialBlock->body.next = NULL;

    /* Initialize the epilogue - block size 0 will be used as a terminating condition */
    block_t* epilogue = (void*) initialBlock + initialBlock->block_size;
    epilogue->allocated = ALLOC;
    epilogue->block_size = 0;

    /* Setting the last segregated free list as the initial block */
    segregatedFreeLists[NUM_SEGREGATED_FREE_LISTS - 1] = initialBlock; /* initialBlock needs to go at the end to not cause a segmentation fault */

    // mm_check();

    return 0;
} /* $end mm_init */

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
/* $begin mm_malloc */
void* mm_malloc(size_t size)
{
    uint32_t alignedSize;    /* Adjusted block size */
    uint32_t sizeExtension;  /* Amount to extend heap if no fit */
    uint32_t wordsExtension; /* Number of words to extend heap if no fit */
    block_t* block;

    /* Ignore spurious requests */
    if (size == 0)
    {
        return NULL;
    }

    /* Adjust block size to include overhead and alignment requirements */
    size += OVERHEAD;

    alignedSize = ((size + 7) >> 3) << 3; /* Align to multiple of 8 */

    if (alignedSize < MIN_BLOCK_SIZE)
    {
        alignedSize = MIN_BLOCK_SIZE;
    }

    /* If the adjusted block size is smaller than the threshold and the heap can extend by an eight of it or its aligned size can fit, it will be placed into one of the segregated free lists */
    if (alignedSize <= SIZE_COMPARE_THRESHOLD && (block = extend_heap(alignedSize >> 3)) != NULL)
    {
        place(block, alignedSize);

        return block->body.payload; 
    } 
    else if ((block = find_fit(alignedSize)) != NULL)
    {
        place(block, alignedSize);

        return block->body.payload;
    }

    /* No fit found. Get more memory and place the block */
    sizeExtension = (alignedSize > CHUNK_SIZE) ? alignedSize : CHUNK_SIZE; /* Extend by the larger of the two */
    wordsExtension = sizeExtension >> 3; /* sizeExtension / 8 */

    if ((block = extend_heap(wordsExtension)) != NULL) {
        place(block, alignedSize);

        return block->body.payload;
    }

    // mm_check();

    /* No more memory */
    return NULL;
} /* $end mm_malloc */

/*
 * mm_free - Free a block
 */
/* $begin mm_free */
void mm_free(void* payload)
{
    block_t* block = payload - sizeof(header_t);
    block->allocated = FREE;

    footer_t* footer = get_footer(block);
    footer->allocated = FREE;

    /* Block must be moved to its appropriate segregated free list before coalescing to prevent segmentation faults */
    insertBlock(block, indexOfSegregatedFreeListToInsert(block->block_size));
    coalesce(block);

    // mm_check();
} /* $end mm_free */

/* The remaining routines are internal helper routines */ 

/*
 * mm_realloc - Naive implementation of mm_realloc
 * NO NEED TO CHANGE THIS CODE!
 */
/* $begin mm_realloc */
void* mm_realloc(void* ptr, size_t size)
{
    void* newp;
    size_t copySize;

    if ((newp = mm_malloc(size)) == NULL)
    {
        printf("ERROR: mm_malloc failed in mm_realloc\n");
        exit(1);
    }

    block_t* block = ptr - sizeof(header_t);
    copySize = block->block_size;

    if (size < copySize)
    {
        copySize = size;
    }

    memcpy(newp, ptr, copySize);
    mm_free(ptr);

    return newp;
} /* $end mm_realloc */

/*
 * mm_checkheap - Check the heap for consistency
 */
/* $begin mm_checkheap */
void mm_checkheap(int verbose)
{
    block_t* block = prologue;

    if (verbose)
    {
        printf("Heap (%p):\n", prologue);
    }

    if (block->block_size != sizeof(header_t) || !block->allocated)
    {
        printf("Bad prologue header\n");
    }

    checkblock(prologue);

    /* Iterate through the heap (Both free and allocated blocks will be present) */
    for (block = (void*) prologue + prologue->block_size; block->block_size > 0; block = (void*) block + block->block_size)
    {
        if (verbose)
        {
            printblock(block);
        }
    
        checkblock(block);
    }

    if (verbose)
    {
        printblock(block);
    }

    if (block->block_size != 0 || !block->allocated)
    {
        printf("Bad epilogue header\n");
    }
} /* $end mm_checkheap */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin extend_heap */
static block_t* extend_heap(size_t words)
{
    block_t* block;
    uint32_t size;

    size = words << 3; /* words * 8 */

    if (size == 0 || (block = mem_sbrk(size)) == (void*) - 1)
    {
        return NULL;
    }

    /* The newly acquired region will start directly after the epilogue block */
    /* Initialize free block header/footer and the new epilogue header */
    /* Use old epilogue as new free block header */
    block = (void*) block - sizeof(header_t);
    block->allocated = FREE;
    block->block_size = size;

    /* Free block footer */
    footer_t* block_footer = get_footer(block);
    block_footer->allocated = FREE;
    block_footer->block_size = block->block_size;

    /* Inserting this new block */
    int index = indexOfSegregatedFreeListToInsert(block->block_size);
    insertBlock(block, index);

    /* New epilogue header */
    header_t* new_epilogue = (void*) block_footer + sizeof(header_t);
    new_epilogue->allocated = ALLOC;
    new_epilogue->block_size = 0;

    /* Only want to coalesce when not checking the aligned size with the size threshold constant in mm_malloc */
    if (words == CHUNK_SIZE / 8)
    {
        return coalesce(block);
    }
    
    return block;
} /* $end extend_heap */

/*
 * place - Place block of alignSize bytes at start of free block block
 *         and split if remainder would be at least minimum block size
 */
/* $begin place */
static void place(block_t* block, size_t alignSize)
{
    size_t splitSize = block->block_size - alignSize;

    /* Remove the old block */
    removeBlock(block, indexOfSegregatedFreeListToInsert(block->block_size));

    if (splitSize >= MIN_BLOCK_SIZE)
    {
        /* Split the block by updating the header and marking it allocated */
        block->block_size = alignSize;
        block->allocated = ALLOC;

        /* Set footer of allocated block*/
        footer_t* footer = get_footer(block);
        footer->block_size = alignSize;
        footer->allocated = ALLOC;

        /* Update the header of the new free block */
        block_t* new_block = (void*) block + block->block_size;
        new_block->block_size = splitSize;
        new_block->allocated = FREE;

        /* Update the footer of the new free block */
        footer_t* new_footer = get_footer(new_block);
        new_footer->block_size = splitSize;
        new_footer->allocated = FREE;

        /* Inserting the new block after updating its footer is ~0.0004 seconds faster than inserting the new block before updating its footer (Spatial locality) */
        insertBlock(new_block, indexOfSegregatedFreeListToInsert(new_block->block_size));
    }
    else
    {
        /* Splitting the block will cause a splinter so we just include it in the allocated block */
        block->allocated = ALLOC;

        footer_t* footer = get_footer(block);
        footer->allocated = ALLOC;
    }
} /* $end place */

/*
 * find_fit - Find a fit for a block with alignSize bytes
 */
/* $begin find_fit */
static block_t* find_fit(size_t alignSize)
{
    for (int index = indexOfSegregatedFreeListToInsert(alignSize); index <= NUM_SEGREGATED_FREE_LISTS - 1; index++)
    {
        for (block_t* b = segregatedFreeLists[index]; b != NULL; b = b->body.next)
        {
            if (!b->allocated && alignSize <= b->block_size)
            {
                return b;
            }
        }
    }

    return NULL; /* No fit */
} /* $end find_fit */

/*
 * coalesce - Boundary tag coalescing, returns pointer to coalesced block
 */
/* $begin coalesce */
static block_t* coalesce(block_t* block)
{
    footer_t* previousFooter = (void*) block - sizeof(header_t);
    header_t* nextHeader = (void*) block + block->block_size;

    bool previousBlockAllocated = previousFooter->allocated;
    bool nextBlockAllocated = nextHeader->allocated;
    
    block_t* nextBlock = (void*) nextHeader;
    block_t* previousBlock = (void*) previousFooter - previousFooter->block_size + sizeof(header_t);

    if (previousBlockAllocated && nextBlockAllocated) /* Case 1 */
    {
        /* No coalescing */
        return block;
    }
    else if (previousBlockAllocated && !nextBlockAllocated) /* Case 2 */
    {
        /* Coalesce the current and next blocks */
        removeBlock(block, indexOfSegregatedFreeListToInsert(block->block_size));
        removeBlock(nextBlock, indexOfSegregatedFreeListToInsert(nextBlock->block_size));

        /* Update header of current block to include next block's size */
        block->block_size += nextHeader->block_size;

        /* Update footer of next block to reflect new size */
        footer_t* next_footer = get_footer(block);
        next_footer->block_size = block->block_size;
    }
    else if (!previousBlockAllocated && nextBlockAllocated) /* Case 3 */
    {
        /* Coalesce the previous and current blocks */
        removeBlock(block, indexOfSegregatedFreeListToInsert(block->block_size));
        removeBlock(previousBlock, indexOfSegregatedFreeListToInsert(previousBlock->block_size));

        /* Update header of prev block to include current block's size */
        previousBlock->block_size += block->block_size;

        /* Update footer of current block to reflect new size */
        footer_t* footer = get_footer(previousBlock);
        footer->block_size = previousBlock->block_size;

        /* Because the previous block was free, the start of the coalesced blocks begins there */
        block = previousBlock;
    }
    else /* Case 4 */
    {
        /* Coalesce the previous, current, and next blocks */
        removeBlock(block, indexOfSegregatedFreeListToInsert(block->block_size));
        removeBlock(nextBlock, indexOfSegregatedFreeListToInsert(nextBlock->block_size));
        removeBlock(previousBlock, indexOfSegregatedFreeListToInsert(previousBlock->block_size));

        /* Update header of prev block to include current and next block's size */
        block_t* previousBlock = (void*) previousFooter - previousFooter->block_size + sizeof(header_t);
        previousBlock->block_size += block->block_size + nextHeader->block_size;

        /* Update footer of next block to reflect new size */
        footer_t* next_footer = get_footer(previousBlock);
        next_footer->block_size = previousBlock->block_size;

        /* Because the previous block was free, the start of the coalesced blocks begins there */
        block = previousBlock;
    }

    /* The newly coalesced block gets added to its appropriate segregated free list */
    insertBlock(block, indexOfSegregatedFreeListToInsert(block->block_size));

    return block;
} /* $end coalesce */

/*
 * get_footer - Returns a pointer to the footer of a block
 */
/* $begin get_footer */
static footer_t* get_footer(block_t* block)
{
    return (void*) block + block->block_size - sizeof(footer_t);
} /* $end get_footer */

/*
 * printblock - Prints the block's header, footer, and whether it is allocated
 */
/* $begin printblock */
static void printblock(block_t* block)
{
    uint32_t headerSize, isHeaderAllocated, footerSize, isFooterAllocated;

    headerSize = block->block_size;
    isHeaderAllocated = block->allocated;

    footer_t* footer = get_footer(block);
    footerSize = footer->block_size;
    isFooterAllocated = footer->allocated;

    if (headerSize == 0)
    {
        printf("%p: EOL\n", block);
        return;
    }

    /* Prints the information of block as well as its previous and next blocks */
    printf("%p: Previous\n", block->body.prev);
    printf("%p: Header: [%d:%c] Footer: [%d:%c]\n", block, headerSize, (isHeaderAllocated ? 'a' : 'f'), footerSize, (isFooterAllocated ? 'a' : 'f'));
    printf("%p: Payload\n", block->body.payload);
    printf("%p: Next\n", block->body.next);
} /* $end printblock */

/*
 * checkblock - Checks the alignment of the block's payload and the header/footer consistency
 */
/* $begin checkblock */
static void checkblock(block_t* block)
{
    if ((uint64_t) block->body.payload % 8)
    {
        printf("Error: payload for block at %p is not aligned\n", block);
    }

    footer_t* footer = get_footer(block);

    if (block->block_size != footer->block_size)
    {
        printf("Error: header does not match footer\n");
    }
} /* $end checkblock */


/* Newly added helper functions*/

/*
 * indexOfSegregatedFreeListToInsert - Returns the index of the segregated free list that the block should be inserted into
 */
/* $begin indexOfSegregatedFreeListToInsert */
static int indexOfSegregatedFreeListToInsert(int blockSize)
{
    int powersOfTwoAbove32 = (32 - 1) - (__builtin_clz(blockSize) + 5); /* Builtin function of gcc to count leading zeros */

    if (powersOfTwoAbove32 >= 0 && powersOfTwoAbove32 < NUM_SEGREGATED_FREE_LISTS - 1)
    {
        return powersOfTwoAbove32;
    }

    return NUM_SEGREGATED_FREE_LISTS - 1; /* The last list contains blocks that don't fit in the first 10 */
} /* $end indexOfSegregatedFreeListToInsert */

/*
 * insertBlock - Inserts a block to the segregated free list
 */
/* $begin insertBlock */
static void insertBlock(block_t* block, int freeListNum)
{
    /* Standard doubly linked list insertion */
    block->body.prev = NULL;

    if (segregatedFreeLists[freeListNum] == NULL) /* List previously empty */
    {
        block->body.next = NULL;
    }
    else /* List not empty, add to start of list */
    {
        block->body.next = segregatedFreeLists[freeListNum];
        segregatedFreeLists[freeListNum]->body.prev = block;
    }

    segregatedFreeLists[freeListNum] = block;
} /* $end insertBlock */

/*
 * removeBlock - Removes a block from the segregated free list
 */
/* $begin removeBlock */
static void removeBlock(block_t* block, int freeListNum)
{
    /* Standard doubly linked list removal */
    block_t* head = segregatedFreeLists[freeListNum];

    if (head->body.prev == NULL && head->body.next == NULL) /* Only block */
    {
        segregatedFreeLists[freeListNum] = NULL;
    }
    else if (block == head) /* First block */
    {
        block->body.next->body.prev = NULL;
        segregatedFreeLists[freeListNum] = block->body.next;
    }
    else if (block->body.next == NULL) /* Last block */
    {
        block->body.prev->body.next = NULL;
    }
    else /* Somewhere in the middle */
    {
        block->body.prev->body.next = block->body.next;
        block->body.next->body.prev = block->body.prev;
    }
} /* $end removeBlock */

/*
 * mm_check - Heap consistency checker
 */
/* $begin mm_check */
static int mm_check(void) /* Makes program take ~0.6 seconds to run, lowers score to 1/100 */
{
    for (block_t* block = (void*) prologue + sizeof(header_t); block->block_size > 0; block = (void*) block + block->block_size) 
    {
        checkblock(block);

        if (!block->allocated) 
        {
            /* Check for contiguous free blocks that weren't coalesced */
            block_t* next_block = (void*) block + block->block_size;

            if (block->allocated == FREE && next_block->allocated == FREE) 
            {
                printf("Contiguous free blocks at %p and %p not coalesced\n", block, next_block);
                printblock(block);

                return -1;
            }
        }
    }

    return 0;
} /* $end mm_check */

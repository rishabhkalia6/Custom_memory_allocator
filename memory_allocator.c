
// RISHABH KALIA 0777461 ASSIGNMENT 3 QUESTION 1 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define the total memory pool size: 1024 * 4 bytes = 4096 */
#define MEMORY_SIZE (1024 * 4)

/* Block header for each memory block in the pool */
typedef struct block_header {
    size_t size;              /* size of the memory block (excluding header) */
    int free;                 /* flag: 1 if free, 0 if allocated */
    struct block_header *next;
} block_header;

/* Global variables for the memory pool and free-list head */
static void *memory_pool = NULL;
static block_header *free_list = NULL;

/* Initialize the memory pool:
   - Allocates MEMORY_SIZE bytes using standard malloc.
   - Sets up the free_list to point to one big free block that covers the entire pool.
*/
void init_memory_pool() {
    memory_pool = malloc(MEMORY_SIZE);
    if (!memory_pool) {
        fprintf(stderr, "Memory pool allocation failed\n");
        exit(1);
    }
    free_list = (block_header*)memory_pool;
    free_list->size = MEMORY_SIZE - sizeof(block_header);
    free_list->free = 1;
    free_list->next = NULL;
}

/* Split a free block into an allocated block and a smaller free block.
   It is called when the found free block is larger than required.
*/
void split_block(block_header *block, size_t size) {
    // new block begins after the allocated portion (header + size)
    block_header *new_block = (block_header*)((char*)block + sizeof(block_header) + size);
    new_block->size = block->size - size - sizeof(block_header);
    new_block->free = 1;
    new_block->next = block->next;
    
    // update current block
    block->size = size;
    block->free = 0;
    block->next = new_block;
}

/* Custom allocation function using First-Fit strategy.
   If a block larger than needed is found, it is split.
   If no block is found, the function first tries to merge adjacent free blocks
   (i.e. defragmentation). If still no block is available, an error is printed.
*/
void *my_malloc(size_t size) {
    block_header *curr = free_list;
    
    // first pass: find a free block that fits using First-Fit.
    while(curr) {
        if(curr->free && curr->size >= size) {
            // If there is room to split (i.e. have room for a new header plus at least 1 byte)
            if(curr->size >= size + sizeof(block_header) + 1) {
                split_block(curr, size);
            } else {
                curr->free = 0; // use the whole block if splitting is not beneficial
            }
            return (void*)((char*)curr + sizeof(block_header));
        }
        curr = curr->next;
    }
    
    // No block found; try defragmentation by merging adjacent free blocks.
    curr = free_list;
    while(curr && curr->next) {
        // if the current block and the next block are physically adjacent, merge them
        if((char*)curr + sizeof(block_header) + curr->size == (char*)curr->next) {
            curr->size += sizeof(block_header) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
    
    // Second pass after merging
    curr = free_list;
    while(curr) {
        if(curr->free && curr->size >= size) {
            if(curr->size >= size + sizeof(block_header) + 1) {
                split_block(curr, size);
            } else {
                curr->free = 0;
            }
            return (void*)((char*)curr + sizeof(block_header));
        }
        curr = curr->next;
    }
    
    // If allocation still fails, print an error message.
    printf("my_malloc: Out of memory, resizing pool not implemented\n");
    return NULL;
}

/* Custom free function.
   It marks the block as free and attempts to merge with adjacent free blocks.
*/
void my_free(void *ptr) {
    if (ptr == NULL)
        return;
    
    // Get the block header from pointer.
    block_header *block = (block_header*)((char*)ptr - sizeof(block_header));
    block->free = 1;
    
    // Merge with next block if it is free and contiguous.
    if (block->next && block->next->free) {
        block->size += sizeof(block_header) + block->next->size;
        block->next = block->next->next;
    }
    
    // Attempt to merge with the previous block (if free and contiguous).
    block_header *curr = free_list;
    while (curr) {
        if (curr->free && curr->next == block) {
            if((char*)curr + sizeof(block_header) + curr->size == (char*)block) {
                curr->size += sizeof(block_header) + block->size;
                curr->next = block->next;
                break;
            }
        }
        curr = curr->next;
    }
}

/* For testing the allocator, we will simulate operations.
   The operations for each test case are given as an array of integers.
   Positive numbers indicate an allocation request of that many bytes.
   Negative numbers indicate that the block corresponding to the absolute value should be freed.
*/
typedef struct {
    int size;    // allocation size (the original positive number)
    void *ptr;   // pointer returned by my_malloc
} Allocation;

/* Function to run one test case. */
void run_test_case(const int ops[], int count) {
    Allocation allocations[20];  // store allocations (assume no more than 20 allocations in a test)
    int alloc_count = 0;         // count of allocation entries

    printf("Running test case: ");
    for (int i = 0; i < count; i++) {
        printf("%d ", ops[i]);
    }
    printf("\n");

    for (int i = 0; i < count; i++) {
        int op = ops[i];
        if (op > 0) {
            // Allocation
            void *p = my_malloc((size_t) op);
            if (p == NULL) {
                printf("Allocation of size %d failed.\n", op);
            } else {
                // record the allocation for later freeing
                allocations[alloc_count].size = op;
                allocations[alloc_count].ptr = p;
                alloc_count++;
                printf("Allocated %d bytes at %p\n", op, p);
            }
        } else { 
            // Deallocation: free the block that was allocated with the same absolute value.
            int target = -op;
            int found = 0;
            for (int j = 0; j < alloc_count; j++) {
                if (allocations[j].size == target && allocations[j].ptr != NULL) {
                    my_free(allocations[j].ptr);
                    printf("Freed %d bytes from %p\n", target, allocations[j].ptr);
                    allocations[j].ptr = NULL; // mark as freed
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("Error: Could not find allocated block for size %d to free!\n", target);
            }
        }
    }
    printf("Test case completed.\n\n");
}

/* Print the free-list status */
void print_free_list() {
    block_header *curr = free_list;
    printf("Free list status:\n");
    while (curr) {
        printf(" Block @ %p: size = %zu, free = %d, next = %p\n",
               (void*)curr, curr->size, curr->free, (void*)curr->next);
        curr = curr->next;
    }
    printf("\n");
}

/* Main driver */
int main() {
    /* Initialize our memory pool */
    init_memory_pool();

    /* Define the 10 test cases; each test case is an array of operations.
       Each test case here has 8 operations (4 allocations and 4 frees) and the operations sum up to 0.
    */
    const int test_cases[10][8] = {
        {1004,  295, 711,  -295, 915, -1004, -711,  -915},
        {842,   156, 678,  -156, 504,  -842,  -678,  -504},
        {320,   731, 928,  -731, 146,  -320,  -928,  -146},
        {562,    83, 410,   -83, 982,  -562,  -410,  -982},
        {721,   612, 875,  -612, 530,  -721,  -875,  -530},
        {417,   908, 267,  -908, 199,  -417,  -267,  -199},
        {134,   658, 901,  -658, 372,  -134,  -901,  -372},
        {741,   502, 819,  -502, 638,  -741,  -819,  -638},
        {298,   420, 555,  -420, 777,  -298,  -555,  -777},
        {612,   203, 890,  -203, 450,  -612,  -890,  -450}
    };

    /* Run each test case independently. */
    for (int t = 0; t < 10; t++) {
        // For clarity, you might reinitialize the pool before each test.
        // (For simplicity we call free(memory_pool) and reinitialize.)
        free(memory_pool);
        init_memory_pool();

        // Run the current test case
        run_test_case(test_cases[t], 8);
        print_free_list();
    }

    /* Clean-up the memory pool (in a real application, you may wish to maintain it longer) */
    free(memory_pool);
    return 0;
}

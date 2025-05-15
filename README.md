# 🧠 Custom Memory  Allocator in C 

This program implements a custom memory allocation system in C using a manually managed memory pool. It mimics the behavior of **``malloc()``** and **``free()``** by allocating memory blocks using the First-Fit strategy. The allocator supports block splitting, coalescing, and defragmentation to handle fragmentation and simulate low-level OS memory management.

---

## 📦 Features 

- Custom **``my_malloc(size_t size)``** using **First-Fit** algorithm
- **``my_free(void *ptr)``** with support for:
  - **Block coalescing**
  - **Merging with previous and next blocks**
- **Simulated defragmentation** pass if no block fits
- Predefined **10 test cases** of allocation/deallocation patterns
- Outputs **free list structure** after each test case
- Designed to mimic low-level OS memory allocation behavior

---

# 🧰 How does it work

The program initializes a fixed-size memory pool and manages it as a linked list of memory blocks. Each block has metadata (size, free flag, and pointer to the next block). When **``my_malloc(size)``** is called, it searches for the first free block large enough to satisfy the request. If the block is larger than needed, it’s split into an allocated block and a smaller free block. When **``my_free(ptr)``** is called, it marks the block as free and attempts to merge it with adjacent free blocks to reduce fragmentation. If no block fits during allocation, the program tries to defragment the memory by merging adjacent free blocks before giving up.

``` C
void *my_malloc(size_t size) {
    block_header *curr = free_list;

    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size >= size + sizeof(block_header) + 1) {
                split_block(curr, size);
            } else {
                curr->free = 0;
            }
            return (void*)((char*)curr + sizeof(block_header));
        }
        curr = curr->next;
    }

    // Defragmentation pass
    // (merging logic shown in the full code)

    printf("my_malloc: Out of memory\n");
    return NULL;
}
```
This snippet shows:
- How First-Fit allocation is done
- When splitting is applied
- The fallback message if no space is found

---

# 🛠️ How to build

``` bash
gcc memory_allocator.c -o allocator
```
In your linux terminal by running this command you can then compile the program and create the executable named **allocator**.

# 💻 Outputs


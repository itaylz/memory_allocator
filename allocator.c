//
// Created by Itay Laznik on 6/12/2024.
//
/*Basic (and first most definitely not the best XD) implementation of a memory allocator allowing
 * a user to request memory from a defined pool and return a pointer to that location to be used
 * by user similar to malloc.
 * Second function free_memory allows memory to be returned to kernel and reused
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//total memory that can be allocated
#define LIMIT 10240
//array of memory that user can allocate from
static uint8_t memory_map[LIMIT];

//metadata structure for memory chunks
typedef struct memory_chunk{
    size_t size;         // Size of the chunk
    bool inuse;          //if chunk is used or not
    struct memory_chunk* next;   //pointer to the next block
} memory_chunk;

//pointer to memory map acting as free_list to show slots available for allocation
static memory_chunk* free_list = (memory_chunk*)memory_map;

//align memory to the size of a pointer for max efficiency
#define ALIGN(size) (((size) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1))

//initialize the memory pool when ran
void init_memory() {
    free_list->size = LIMIT - sizeof(memory_chunk);
    free_list->inuse = true;
    free_list->next = NULL;
}

void *alloc(size_t size) {
    if (size == 0 || size > LIMIT) {
        printf("Invalid input size\n");
    }

    size = ALIGN(size); //align the input size

    memory_chunk *allocator = free_list; //iterator to free_list

    //traverse free list to find available block of memory to allocate to user
    while (allocator != NULL) {
        if (allocator->inuse && allocator->size >= size) {
            //if the block is large enough, split it
            if (allocator->size > size + sizeof(memory_chunk )) {
                memory_chunk * new_chunk = (memory_chunk *)((uint8_t*)allocator +
                                            sizeof(memory_chunk ) + size);

                new_chunk->size = allocator->size - size - sizeof(memory_chunk );
                new_chunk->inuse = true;
                new_chunk->next = allocator->next;

                allocator->size = size;
                allocator->next = new_chunk;
            }
            allocator->inuse = false;
            //pointer to successfully allocated chunk returned to user
            return (void*)((uint8_t*)allocator + sizeof(memory_chunk ));
        }
        allocator = allocator->next; //allocator iterates to next chunk
    }
    return NULL; //if no available block to allocate found
}

//function for freeing chunk
void free_memory(void *ptr) {
    if (ptr == NULL) {
        printf("Freeing block unsuccessful: Pointer is NULL");
    }

    memory_chunk *chunk = (memory_chunk*)((uint8_t*)ptr - sizeof(memory_chunk));
    chunk->inuse = true;

    //merge consecutive free blocks
    memory_chunk * allocator = free_list;
    while (allocator != NULL && allocator->next != NULL) {
        if (allocator->inuse && allocator->next->inuse) {
            allocator->size += sizeof(memory_chunk ) + allocator->next->size;
            allocator->next = allocator->next->next;
        } else {
            allocator = allocator->next;
        }
    }
}

//function to print the memory block allocated
void print_memory() {
    memory_chunk *iterator = free_list;
    while (iterator != NULL) {
        printf("Start: %p | Size: %zu | Free: %s\n",
               (void*)iterator,
               iterator->size,
               iterator->inuse ? "Yes" : "No");
        iterator = iterator->next;
    }
}


int main(void) {
    init_memory();

    printf("Initial memory status:\n");
    print_memory();

    void *ptr1 = alloc(200);
    printf("after initial allocation:\n");
    print_memory();

    void *ptr2 = alloc(100);
    printf("after second allocation:\n");
    print_memory();

    free_memory(ptr1);
    free_memory(ptr2);
    print_memory();

    return 0;
}
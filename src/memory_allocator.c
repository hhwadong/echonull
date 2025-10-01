#include "../include/memory_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MEMORY_POOL_SIZE 4096
#define ALIGNMENT 8

static char memoryPool[MEMORY_POOL_SIZE];
static MemoryBlock *head = NULL;
static bool initialized = false;

static size_t alignSize(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

void initializeMemory(void) {
    if (initialized) {
        return;
    }

    head = (MemoryBlock *)memoryPool;
    head->size = MEMORY_POOL_SIZE - sizeof(MemoryBlock);
    head->free = true;
    head->next = NULL;

    initialized = true;
    printf("Memory allocator initialized with %d bytes\n", MEMORY_POOL_SIZE);
}

void *myMalloc(size_t size) {
    if (!initialized) {
        initializeMemory();
    }

    if (size == 0) {
        return NULL;
    }

    size = alignSize(size);
    MemoryBlock *current = head;

    while (current != NULL) {
        if (current->free && current->size >= size) {
            if (current->size > size + sizeof(MemoryBlock) + ALIGNMENT) {
                MemoryBlock *newBlock = (MemoryBlock *)((char *)current + sizeof(MemoryBlock) + size);
                newBlock->size = current->size - size - sizeof(MemoryBlock);
                newBlock->free = true;
                newBlock->next = current->next;

                current->size = size;
                current->next = newBlock;
            }

            current->free = false;
            return (char *)current + sizeof(MemoryBlock);
        }
        current = current->next;
    }

    return NULL;
}

void myFree(void *ptr) {
    if (ptr == NULL || !initialized) {
        return;
    }

    MemoryBlock *block = (MemoryBlock *)((char *)ptr - sizeof(MemoryBlock));

    if (block < (MemoryBlock *)memoryPool ||
        (char *)block >= memoryPool + MEMORY_POOL_SIZE) {
        return;
    }

    block->free = true;

    MemoryBlock *current = head;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            current->size += current->next->size + sizeof(MemoryBlock);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void visualizeMemory(void) {
    if (!initialized) {
        printf("Memory allocator not initialized\n");
        return;
    }

    printf("\n=== Memory Visualization ===\n");
    MemoryBlock *current = head;
    int blockNum = 1;
    size_t usedSpace = 0;
    size_t freeSpace = 0;

    while (current != NULL) {
        char status = current->free ? 'F' : 'U';
        size_t totalSize = current->size + sizeof(MemoryBlock);

        printf("Block %d: %c (Size: %zu bytes, Data: %zu bytes)\n",
               blockNum++, status, totalSize, current->size);

        if (current->free) {
            freeSpace += current->size;
        } else {
            usedSpace += current->size;
        }
        current = current->next;
    }

    printf("\nSummary: Used: %zu bytes, Free: %zu bytes, Total: %d bytes\n",
           usedSpace, freeSpace, MEMORY_POOL_SIZE);
    printf("Fragmentation: %.2f%%\n",
           freeSpace > 0 ? (float)(freeSpace - (freeSpace / (freeSpace / usedSpace + 1))) * 100 / freeSpace : 0);
}

void analyzeMemory(size_t *totalFree, size_t *largestBlock, int *fragmentCount) {
    if (!initialized) {
        *totalFree = 0;
        *largestBlock = 0;
        *fragmentCount = 0;
        return;
    }

    *totalFree = 0;
    *largestBlock = 0;
    *fragmentCount = 0;

    MemoryBlock *current = head;
    while (current != NULL) {
        if (current->free) {
            *totalFree += current->size;
            (*fragmentCount)++;
            if (current->size > *largestBlock) {
                *largestBlock = current->size;
            }
        }
        current = current->next;
    }
}
#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <stdbool.h>
#include <stddef.h>

typedef struct MemoryBlock {
    size_t size;
    bool free;
    struct MemoryBlock *next;
} MemoryBlock;

void initializeMemory(void);
void *myMalloc(size_t size);
void myFree(void *ptr);
void visualizeMemory(void);
void analyzeMemory(size_t *totalFree, size_t *largestBlock, int *fragmentCount);

#endif
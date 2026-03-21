#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef MILA_DEBUG
    #include <stdio.h>
#endif

// change this to limit or expand memory
// on embedded builds
#define SIZE_MB 10

#define HEAP_SIZE (SIZE_MB * 1024 * 1024) // 2 MB
#define ALIGN 4

typedef struct BlockHeader {
    size_t size;
    struct BlockHeader* next; // only used if block is free
} BlockHeader;
typedef BlockHeader FreeBlock;

static uint8_t heap[HEAP_SIZE];
static size_t heap_used = 0;
static FreeBlock* free_list = NULL;

// align up to next multiple of ALIGN
size_t align_up(size_t size) {
    return (size + (ALIGN - 1)) & ~(ALIGN - 1);
}

// insert freed block and coalesce neighbors
void insert_free_block(FreeBlock* block) {
    FreeBlock** prev = &free_list;
    FreeBlock* curr = free_list;

    while (curr && curr < block) {
        prev = &curr->next;
        curr = curr->next;
    }

    block->next = curr;
    *prev = block;

    // coalesce with next
    if (curr && (uint8_t*)block + block->size == (uint8_t*)curr) {
        block->size += curr->size;
        block->next = curr->next;
    }

    // coalesce with previous
    if (prev != &free_list) {
        FreeBlock* prev_block = (FreeBlock*)((uint8_t*)prev - offsetof(FreeBlock, next));
        if ((uint8_t*)prev_block + prev_block->size == (uint8_t*)block) {
            prev_block->size += block->size;
            prev_block->next = block->next;
        }
    }
}

void* mila_malloc(size_t size) {
    size = align_up(size);
    size_t total_size = size + sizeof(BlockHeader);

    // check free list first
    FreeBlock** prev = &free_list;
    FreeBlock* block = free_list;
    while (block) {
        if (block->size >= total_size) {
            *prev = block->next;
            BlockHeader* header = (BlockHeader*)block;
            header->size = block->size;
            return (void*)(header + 1);
        }
        prev = &block->next;
        block = block->next;
    }

    // bump allocation
    if (heap_used + total_size > HEAP_SIZE) return NULL;
    BlockHeader* header = (BlockHeader*)(heap + heap_used);
    header->size = total_size;
    heap_used += total_size;
#ifdef MILA_DEBUG
    printf("  ?? mila_malloc: allocated %zu at %p\n", size, header + 1);
#endif
    return (void*)(header + 1);
}

void mila_free(void* ptr) {
    if (!ptr) return;
    BlockHeader* header = ((BlockHeader*)ptr) - 1;
    FreeBlock* block = (FreeBlock*)header;
    block->size = header->size;
    block->next = NULL; // IMPORTANT
#ifdef MILA_DEBUG
    printf("  ?? mila_free: freed %zu at %p\n", header->size, ptr);
#endif
    insert_free_block(block);
}

void* mila_realloc(void* ptr, size_t new_size) {
    if (!ptr) return mila_malloc(new_size); // realloc behaves like malloc
    if (new_size == 0) {
        mila_free(ptr);
        return NULL;
    }

    BlockHeader* header = ((BlockHeader*)ptr) - 1;
    size_t old_size = header->size - sizeof(BlockHeader);
    new_size = align_up(new_size);

    // if shrinking, just leave extra space (optional: could free remaining)
    if (new_size <= old_size) {
        return ptr;
    }

    // allocate new block
    void* new_ptr = mila_malloc(new_size);
    if (!new_ptr) return NULL;

    // copy old data
    memcpy(new_ptr, ptr, old_size);
    mila_free(ptr);
    return new_ptr;
}

void mila_reset() {
    heap_used = 0;
    free_list = NULL;
}
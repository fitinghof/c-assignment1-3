#include "memory_manager.h"

void *memory_;
void *memory_end;
size_t space_left = 0;

#define block_size_mask 0xfffffffC
#define block_free_mask 1
#define align_size 4
#define ALIGN(a) (a + align_size - 1) & ~(align_size - 1)


typedef uint32_t header;

size_t block_size(header *block) { return *block & block_size_mask; }

bool block_isfree(header *block) { return *block & block_free_mask; }

void block_set_free(header *block, bool free) {
    if (free)
        *block = *block | 1;
    else
        *block = *block & ~(uint16_t)1;
}

void block_set_size(header *block, uint32_t size) {
    *block = (*block & 1) | size;
}

uint32_t *block_get_next(header *block) {
    return ((void *)block) + (*block & block_size_mask) + sizeof(header);
}

/// @brief makes a qualified guess wether or not the block is valid, can never
/// fail to identify a valid block
/// @param block
/// @return
bool block_is_valid(header *block) {
    while (block < memory_end) {
        block = block_get_next(block);
        if (block == memory_end) return true;
    }
    return false;
}

/// @brief loads up the memory with memory
/// @param size size in bytes
void mem_init(size_t size) {
    size = ALIGN(size);
    size_t total_size = size + sizeof(header) * 17;
    memory_ = malloc(total_size);
    memory_end = memory_ + total_size;
    header *initial_block = memory_;
    block_set_size(initial_block, total_size - sizeof(header));
    block_set_free(initial_block, true);
    space_left = size;
}

/// @brief returns pointer to memory block, NULL if no chunk of proper size
/// found
/// @param size size in bytes
/// @return
void *mem_alloc(size_t size) {
    if(size > space_left) return NULL;
    if(size == 0) return memory_ + sizeof(header);
    size = ALIGN(size);
    header *first_free_block = NULL;
    header *walker = memory_;
    size_t continius_memory = 0;
    while (walker != memory_end) {
        while (!block_isfree(walker)) {
            walker = block_get_next(walker);
            if (walker == memory_end) return NULL;
        }
        first_free_block = walker;
        while (walker != memory_end && block_isfree(walker)) {
            continius_memory += block_size(walker) + sizeof(header);
            walker = block_get_next(walker);
        }
        if (continius_memory - sizeof(header) >= size && first_free_block) {
            if (continius_memory - sizeof(header) < size + sizeof(header)) {
                block_set_size(first_free_block,
                               continius_memory - sizeof(header));
                block_set_free(first_free_block, false);

                space_left -= size;
                return first_free_block + 1;
            }
            block_set_size(first_free_block, size);
            block_set_free(first_free_block, false);

            header *new_block = block_get_next(first_free_block);
            block_set_size(new_block,
                           continius_memory - size - 2 * sizeof(header));
            block_set_free(new_block, true);

            space_left -= size;
            return first_free_block + 1;
        }
        continius_memory = 0;
        first_free_block = NULL;
    }
    return NULL;
}

/// @brief Frees the memory block preventing memory leaks
/// @param block block to free
void mem_free(void *block) {
    if (!block) return;
    if (!block_is_valid(block - sizeof(header))) return;
    header *block_header = block - sizeof(header);
    block_set_free(block_header, true);
    space_left += block_size(block_header);
}

/// @brief changes the size of the block, if possible without moving it, returns
/// NULL if failed
/// @param block block to resize
/// @param size size in bytes
/// @return pointer to resized block, NULL if failed
void *mem_resize(void *block, size_t size) {
    if (block == NULL) return mem_alloc(size);
    if (!block_is_valid(block - sizeof(header))) return NULL;
    if (size == 0) {
        mem_free(block);
        return NULL;
    }
    header *header = block - sizeof(*header);
    block_set_free(header, true);
    void *new_block = mem_alloc(size);
    if (!new_block) {
        block_set_free(header, false);
        return NULL;
    }
    size_t minsize = (block_size(header) < size) ? block_size(header) : size;
    memcpy(new_block, ((void *)header) + sizeof(*header), minsize);
    return new_block;
}

/// @brief returns the memory used by the memory manager
void mem_deinit() { free(memory_); }
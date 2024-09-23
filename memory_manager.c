#include "memory_manager.h"

unsigned char *block_starts_;
unsigned char *block_ends_;
void *start_;
size_t size_;

/// @brief sets a bit to 1
/// @param array
/// @param index
void set_bit(unsigned char *array, size_t index) {
    array[index / 8] |= (1 << (index % 8));
}

/// @brief sets a bit to 0
/// @param array
/// @param index
void clear_bit(unsigned char *array, size_t index) {
    array[index / 8] &= ~(1 << (index % 8));
}

/// @brief returns bit
/// @param array
/// @param index
/// @return
bool get_bit(const unsigned char *array, size_t index) {
    return array[index / 8] & (1 << (index % 8));
}

/// @brief loads up the memory with memory
/// @param size
void mem_init(size_t size) {
    start_ = malloc(size);
    block_starts_ = calloc((size + 7) / 8, 1);
    block_ends_ = calloc((size + 7) / 8, 1);
    size_ = size;
}

/// @brief returns pointer to memory block, NULL if no chunk of proper size
/// found
/// @param size
/// @return
void *mem_alloc(size_t size) {
    //Why should alloc of size 0 return anything besides NULL? This is stupid.
    if (size == 0) return 69;
    size_t current_empty_blocks = 0;
    bool empty = true;
    for (size_t i = 0; i < size_; i++) {
        if (get_bit(block_starts_, i) == 1) empty = false;
        current_empty_blocks = (empty) ? current_empty_blocks + 1 : 0;

        if (current_empty_blocks == size) {
            set_bit(block_ends_, i);
            set_bit(block_starts_, i + 1 - size);
            return start_ + (i + 1 - size);
        }
        if (get_bit(block_ends_, i) == 1) empty = true;
    }
    return NULL;
}

void *mem_alloc_bestfit(size_t size) {
    if (size == 0) return NULL;
    size_t current_empty_blocks = 0;
    bool empty = true;
    void *bestfit = NULL;
    size_t smallest_size = -1;
    size_t index = 0;
    while (index < size_) {
        if (get_bit(block_starts_, index) == 1) {
            empty = false;
            if (current_empty_blocks >= size &&
                current_empty_blocks < smallest_size) {
                bestfit = start_ + index - current_empty_blocks;
                smallest_size = current_empty_blocks;
            }
            current_empty_blocks = 0;
        }
        if (empty) current_empty_blocks++;
        if (get_bit(block_ends_, index) == 1) empty = true;
        index++;
    }
    if (current_empty_blocks >= size && current_empty_blocks < smallest_size) {
        bestfit = start_ + index - current_empty_blocks;
        smallest_size = current_empty_blocks;
    }
    if (bestfit) {
        set_bit(block_starts_, bestfit - start_);
        set_bit(block_ends_, bestfit - start_ + size - 1);
        return bestfit;
    }
    return NULL;
}

/// @brief Frees the memory block preventing memory leaks
/// @param block
void mem_free(void *block) {
    size_t block_index = block - start_;
    if (block_index >= size_ || get_bit(block_starts_, block_index) == 0 ||
        block == NULL)
        return;

    clear_bit(block_starts_, block_index);
    while (get_bit(block_ends_, block_index) == 0) block_index++;
    clear_bit(block_ends_, block_index);
}

/// @brief changes the size of the block, if possible without moving it, returns
/// NULL if failed
/// @param block
/// @param size
/// @return
void *mem_resize(void *block, size_t size) {
    if (block == NULL) return mem_alloc(size);
    size_t start_index = block - start_;
    if (start_index >= size_ || !get_bit(block_starts_, start_index)) return NULL;

    size_t end_index = start_index;
    while(!get_bit(block_ends_, end_index)) end_index++;

    mem_free(block);
    if (size == 0) return NULL;

    void* new_block = mem_alloc(size);
    if(!new_block) {
        set_bit(block_starts_, start_index);
        set_bit(block_ends_, end_index);
        return NULL;
    }
    if(new_block == block) return block;

    size_t old_size = end_index - start_index + 1;
    size_t min_size = (size < old_size) ? size : old_size;
    memcpy(new_block, block, min_size);
    return new_block;
}

/// @brief gives back the memory used by the memory manager
void mem_deinit() {
    free(block_ends_);
    free(block_starts_);
    free(start_);
}
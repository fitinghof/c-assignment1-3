#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "memory_manager.h"

void test_memory_validity_(int **block_array, size_t nrOf_blocks,
                           size_t block_size) {
    printf_yellow("  Validating block(s)\n");

    printf_yellow("  Checking if allocation was successful\n");
    for (size_t i = 0; i < nrOf_blocks; i++) {
        if (!block_array[i]) {
            printf_red("  -Block %ld allocation failed!", i + 1);
            my_assert(false);
        }
    }
    printf_green("  -Successfull\n");

    printf_yellow("  Filling all blocks and checking expected values\n");
    for (size_t i = 0; i < block_size; i++) {
        for (size_t j = 0; j < nrOf_blocks; j++) {
            block_array[j][i] = i + j * nrOf_blocks;
        }
    }
    for (size_t i = 0; i < block_size; i++) {
        for (size_t j = 0; j < nrOf_blocks; j++) {
            if (block_array[j][i] != i + j * nrOf_blocks) {
                printf_red(
                    "  -Block %ld did not contain the expected value at index "
                    "%ld, expected vale: %ld, actual value: %d, most likely "
                    "due to memory-block overlap\n",
                    j, i, i + j * nrOf_blocks, block_array[j][i]);
                my_assert(false);
            }
        }
    }
    printf_green("  -Successfull validation passed\n");
}

void mem_free_all(void **blocks, size_t nrOf_blocks) {
    for (size_t i = 0; i < nrOf_blocks; i++) mem_free(blocks[i]);
}

void test_init() {
    printf_yellow("Testing mem_init...\n");
    static const size_t block_size = 124;

    // Initialize with block_size * 3) bytes of memory
    // Try allocating to check if init was successful (block_size * 4 bytes)

    int *blocks[3];
    mem_init(sizeof(**blocks) * block_size * 3);
    blocks[0] = mem_alloc(sizeof(**blocks) * block_size);
    blocks[1] = mem_alloc(sizeof(**blocks) * block_size);
    blocks[2] = mem_alloc(sizeof(**blocks) * block_size);

    // Test that allocation was successfull

    test_memory_validity_(blocks, 3, block_size);

    mem_free_all((void **)blocks, 3);
    mem_deinit();
    printf_green("mem_init passed.\n");
}

void test_alloc_and_free() {
    printf_yellow("Testing mem_alloc and mem_free...\n");
    static const size_t nrOf_blocks = 16;
    static const size_t block_size = 16;
    int *blocks[nrOf_blocks];
    static const size_t memory_size =
        nrOf_blocks * sizeof(**blocks) * block_size;

    printf_yellow("  Trying to create %ld blocks of size %ld\n", nrOf_blocks,
                  block_size);
    mem_init(memory_size);
    for (size_t i = 0; i < nrOf_blocks; i++) {
        blocks[i] = mem_alloc(sizeof(**blocks) * block_size);
        my_assert(blocks[i] != NULL);
    }
    printf_green("  -Successfull\n");

    test_memory_validity_(blocks, nrOf_blocks, block_size);

    printf_yellow("  Trying allocation one over size limit\n");
    char *extra_block = mem_alloc(sizeof(*extra_block));
    my_assert(extra_block == NULL &&
              "  -Allocation over size limit did not fail when it should have");
    printf_green("  -Passed\n");

    printf_yellow("  Freeing all blocks and attempting reallocation\n");
    for (size_t i = 0; i < nrOf_blocks; i++) mem_free(blocks[i]);
    blocks[0] = mem_alloc(memory_size);
    my_assert(blocks[0] != NULL &&
              "  -Memory reallocation failed, mem_free likely did not free all "
              "memory properly");
    printf_green("  -Successfull\n");

    printf_yellow("  Filling reallocated space\n");
    for (size_t i = 0; i < block_size * nrOf_blocks; i++) blocks[0][i] = i;
    printf_green("  -Successfull\n");

    mem_free_all((void**)blocks, nrOf_blocks);

    printf_yellow("  Testing allocation of size 0\n");
    my_assert(mem_alloc(0) == NULL && "mem_alloc(0) did not return NULL");
    printf_green("  -Successfull\n");

    mem_deinit();
    printf_green("mem_alloc and mem_free passed.\n");
}

void test_resize() {
    printf_yellow("Testing mem_resize...\n");
    int *block;
    static const size_t block_size = 16;
    static const size_t nrOf_blocks = 16;
    assert(nrOf_blocks > 1 && "Number of blocks cannot be less than 2!");
    static const size_t memory_size = sizeof(*block) * block_size * nrOf_blocks;
    mem_init(memory_size);

    printf_yellow(
        "  Testing resize from half memory size to full memory size\n");
    block = mem_alloc(sizeof(*block) * (block_size / 2));
    my_assert(block != NULL && "Failed to allocate block");
    block = mem_resize(block, memory_size);
    my_assert(block != NULL && "Block resize to full memory failed");
    printf_green("  -Successfull\n");

    printf_yellow("  Testing resize to size 0\n");
    block = mem_resize(block, 0);
    my_assert(block == NULL && "Block resize to 0 did not return NULL");
    printf_green("  -Successfull\n");

    printf_yellow("  Testing if resize to 0 properly freed memory\n");
    block = mem_alloc(memory_size);
    my_assert(
        block != NULL &&
        "block allocation failed, mem_resize to 0 might not have freed memory");
    mem_free(block);
    printf_green("  -Successfull\n");

    printf_yellow("  Testing resize with invalid pointer\n");
    block = mem_resize(block, block_size);
    my_assert(block == NULL &&
              "Block resize with invalid pointer did not return NULL");
    printf_green("  -Successfull\n");

    printf_yellow("  Testing resize with NULL block\n");
    block = mem_resize(NULL, block_size);
    my_assert(block != NULL &&
              "Block resize with NULL block did not allocate new block");
    mem_free(block);
    printf_green("  -Successfull\n");

    printf_yellow(
        "  Testing resize with an empty block inbetween a bunch of other "
        "blocks\n");
    int *blocks[nrOf_blocks];
    for (size_t i = 0; i < nrOf_blocks; i++)
        blocks[i] = mem_alloc(sizeof(**blocks) * block_size);

    size_t resized_block_index = nrOf_blocks / 2 - 1;
    size_t removed_block_index = nrOf_blocks / 2;
    mem_free(blocks[removed_block_index]);

    block = mem_resize(blocks[resized_block_index],
                       sizeof(**blocks) * block_size * 3);
    my_assert(block == NULL &&
              "Block resize to invalid size succeded when it shouldn't");

    blocks[resized_block_index] = mem_resize(blocks[resized_block_index],
                                             sizeof(**blocks) * block_size * 2);
    my_assert(blocks[resized_block_index] != NULL &&
              "Block resize to valid size failed");
    printf_green("  -Successfull\n");
    mem_free_all((void **)blocks, nrOf_blocks);

    mem_deinit();
    printf_green("mem_resize passed.\n");
}

void test_allocation_exceeding_memory_size() {
    printf_yellow("Testing allocations exceeding memory pool size\n");
    static const size_t nrOf_blocks = 8;
    assert(nrOf_blocks > 2);

    static const size_t block_size = 16;
    int *blocks[nrOf_blocks];
    static const size_t memory_size =
        sizeof(**blocks) * (nrOf_blocks - 1) * block_size;

    mem_init(memory_size);  // Initialize with memory_size of memory
    blocks[0] = mem_alloc(memory_size + 1);
    my_assert(blocks[0] == NULL &&
              "Successfully allocated more memory than memory pool size, this "
              "is bad");

    blocks[0] = mem_alloc(memory_size);
    my_assert(blocks[0] != NULL &&
              "failed to allocate exact memory in memory pool");

    blocks[1] = mem_alloc(1);  // This should fail, no space left
    my_assert(blocks[1] == NULL &&
              "Allocated one more than total memory pool, not good");

    mem_free(blocks[0]);

    for (size_t i = 0; i < nrOf_blocks; i++)
        blocks[i] = mem_alloc(sizeof(**blocks) * block_size);

    my_assert(blocks[nrOf_blocks - 1] == NULL &&
              "Succeded with allocation beyond memory capacity, bad");

    mem_free_all((void**)blocks, nrOf_blocks - 1);

    mem_deinit();
    printf_green("Allocations exceeding pool size test passed.\n");
}

void test_double_free() {
    printf_yellow("Testing double deallocation...\n");
    mem_init(1024);  // Initialize with 1KB of memory

    void *block1 = mem_alloc(512);  // Allocate a block of 100 bytes
    my_assert(block1 != NULL);      // Ensure the block was allocated

    void *block2 = mem_alloc(512);  // Allocate a block of 100 bytes
    my_assert(block2 != NULL);      // Ensure the block was allocated

    mem_free(block1);  // Free the block for the first time
    mem_free(block1);  // Attempt to free the block a second time

    my_assert(mem_alloc(1024) == NULL && "Double mem_free of the same block freed another block");
    block1 = mem_alloc(512);
    my_assert(block1 != NULL && "Failed to alocate memory again after double free");

    mem_free(block1);
    mem_free(block2);
    mem_free(block2);

    printf_green(
        "Double deallocation test passed (if no crash and handled "
        "gracefully).\n");
    mem_deinit();  // Cleanup memory
}

void test_memory_fragmentation() {
    printf_yellow("Testing memory fragmentation handling...\n");

    static const size_t nrOf_blocks = 4;
    static const size_t block_size = 8;
    int *blocks[nrOf_blocks];

    static const size_t memory_size =
        sizeof(**blocks) * block_size * nrOf_blocks;

    mem_init(memory_size);  // Initialize with 2048 bytes

    for (size_t i = 0; i < nrOf_blocks; i++)
        blocks[i] = mem_alloc(sizeof(**blocks) * block_size);

    mem_free(blocks[2]);  // Free third block, leaving a fragmented hole after block2
    blocks[2] = mem_alloc(sizeof(**blocks) * block_size);  // Should fit into the space of block3
    assert(blocks[2] != NULL);

    mem_free(blocks[0]);
    mem_free(blocks[2]);
    mem_free(blocks[3]);

    blocks[3] = mem_alloc(sizeof(**blocks) * block_size * 2); // should fit after second block
    blocks[0] = mem_alloc(sizeof(**blocks) * block_size); // should fit before second block

    mem_free_all((void**)blocks, nrOf_blocks);

    mem_deinit();
    printf_green("Memory fragmentation test passed.\n");
}

int main() {
    test_init();
    test_alloc_and_free();
    test_resize();
    test_allocation_exceeding_memory_size();
    test_double_free();
    test_memory_fragmentation();
    printf_green("All tests passed successfully!\n");
    return 0;
}
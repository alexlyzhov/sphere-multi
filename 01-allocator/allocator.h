#ifndef ALLOCATOR
#define ALLOCATOR
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include "allocator_pointer.h"
#include "allocator_error.h"
#include "block.h"

/**
 * Wraps given memory area and provides defagmentation allocator interface on
 * the top of it.
 *
 *
 */
class Allocator {
    void *mem;
    size_t memsize;
    std::vector<Block> blocks;
public:
    Allocator(void *base, size_t size);

    Pointer alloc(size_t N);

    void realloc(Pointer& p, size_t N);

    void free(Pointer& p);

    void defrag();

    void cout_dump();

    void merge(std::vector<Block>::iterator it);

    Pointer get_pointer(Block block);

    void *get_memory(int id);
};

#endif // ALLOCATOR

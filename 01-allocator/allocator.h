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
    int memsize;
public:
    Allocator(void *base, int size);

    Block get_first();

    bool is_last(Block block);

    bool is_first(Block block);

    Block next_block(Block block);

    Block get_next(Block orig_block);

    Block get_prev(Block orig_block);

    Pointer alloc(int N); // check N

    void free(Pointer& p);

    void cout_dump();

    void realloc(Pointer &p, int N);

    void defrag();

    void merge(Block block);

    Pointer get_pointer(Block block);

    Block get_block(Pointer pointer);

    void *get_memory(int id);
};

#endif // ALLOCATOR

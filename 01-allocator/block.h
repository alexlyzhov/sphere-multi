#ifndef BLOCK
#define BLOCK

#include <cstdlib>
#include "allocator_pointer.h"

class Block {
	static int last_id;
	int id; // -1 when the block is free
public:
	void *mem;
	size_t start;
	size_t size;

	Block(void *mem, size_t start, size_t size, bool free);

	void generate_id();

	void set_id(int id);

	void free();

	int get_id();

	bool is_free();

	void *get();
};

#endif // BLOCK

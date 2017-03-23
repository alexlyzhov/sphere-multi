#ifndef BLOCK
#define BLOCK

#include <cstdlib>
#include "allocator_pointer.h"

class Block {
	static int last_id;
	int id; // -1 when the block is free
	bool valid = true;
	int start; // start of metadata
	int size; // meta_size + data_size

	void set_id(int id);

	void write();
public:
	static const int meta_size;
	static const void *mem;

	Block(int start, int size, bool free);

	Block(int start);

	Block();

	void generate_id();

	void free();

	int get_id();

	bool is_free();

	void *get();

	int data_size();

	bool is_valid();

	int get_start();

	void move_metadata(int new_start);

	void move_and_free(int new_start);

	int get_size();

	void set_size(int new_size);

	void set_data_size(int new_size);

	void cout_dump();
};

#endif // BLOCK

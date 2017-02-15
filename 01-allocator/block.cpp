#include "block.h"

int Block::last_id = 0;

Block::Block(void *mem, size_t start, size_t size, bool free) : mem(mem), start(start), size(size) {
	if (free) {
		id = -1;
	} else {
		generate_id();
	}
}

void Block::generate_id() {
	last_id++;
	id = last_id;
}

void Block::free() {
	id = -1;
}

void Block::set_id(int id) {
	this->id = id;
}

bool Block::is_free() {
	return (id == -1);
}

int Block::get_id() {
	return id;
}

void *Block::get() {
	return (char *) mem + start;
}
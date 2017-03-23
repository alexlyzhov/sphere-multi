#include "block.h"
#include <iostream>
#include <cstring>

const int Block::meta_size = sizeof(int) * 2;
int Block::last_id = 0;
const void *Block::mem;

Block::Block(int start, int data_size, bool free) : start(start), size(data_size + meta_size) {
	// if there's no valid block at start
	if (free) {
		id = -1;
	} else {
		generate_id();
	}
	write();
}

Block::Block(int start) : start(start) {
	// read a valid block info from memory
	size = *(int *)((char *) mem + start);
	// std::cout << "read size " << size << " from addr " << (long) ((char *) mem + start) << std::endl;
	id = *((int *)((char *) mem + start) + 1);
	// std::cout << "read id " << id << " from addr " << (long) ((int *)((char *) mem + start) + 1) << std::endl;
}

Block::Block() {
	valid = false;
}

void Block::write() {
	// std::cout << "write size " << size << " to addr " << (long) ((char *) mem + start) << std::endl;
	// std::cout << size << std::endl;
	// std::cout << *((int *) mem) << std::endl;
	*(int *)((char *) mem + start) = size;
	// std::cout << *((int *) mem + start) << " - after" << std::endl;
	// std::cout << "after write: size " << (int) *((char *) mem) << " from addr " << (long) ((char *) mem + start) << std::endl;
	*((int *)((char *) mem + start) + 1) = id;
}

void Block::generate_id() {
	last_id++;
	set_id(last_id); // does write()
}

void Block::free() {
	id = -1;
	write();
}

void Block::set_id(int id) {
	this->id = id;
	write();
}

bool Block::is_free() {
	return (id == -1);
}

int Block::get_id() {
	return id;
}

void *Block::get() {
	return (char *) mem + start + meta_size;
}

int Block::data_size() {
	return size - meta_size;
}

bool Block::is_valid() {
	return valid;
}

int Block::get_start() {
	return start;
}

void Block::move_metadata(int new_start) {
	start = new_start;
	write();
}

void Block::move_and_free(int new_start) {
	int old_id = get_id();
	free();
	int old_start = start;
	move_metadata(new_start);
	std::memcpy(get(), (char *) mem + old_start + meta_size, data_size());
	set_id(old_id);
}

int Block::get_size() {
	return size;
}

void Block::set_size(int new_size) {
	size = new_size;
	write();
}

void Block::set_data_size(int new_size) {
	set_size(new_size + meta_size);
}

void Block::cout_dump() { // info: start, size, id
	std::cout << get_start() << ", " << get_size() << ", " << get_id() << std::endl;
}
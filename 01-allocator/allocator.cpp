#include "allocator.h"
#include "allocator_pointer.h"
#include "allocator_error.h"
#include "block.h"
#include <iostream>
#include <cstring>

Allocator::Allocator(void *base, int size) : mem(base), memsize(size) { // check size
	Block::mem = mem;
	Block init_block = Block(0, memsize - Block::meta_size, true);
}

Block Allocator::get_first() {
	Block first = Block(0);
	return first;
}

bool Allocator::is_last(Block block) {
	if (block.get_start() + block.get_size() == memsize) {
		return true;
	} else if (block.get_start() + block.get_size() > memsize) {
		// std::cout << block.get_start() << " + " << block.get_size() << " = " << block.get_start() + block.get_size() << " -- " << memsize << std::endl;
		std::cout << "Error: incorrect memory layout in Block::is_last()" << std::endl;
		return true;
	}
	return false;
}

bool Allocator::is_first(Block block) {
	if (block.get_start() == 0) {
		return true;
	}
	return false;
}

Block Allocator::get_prev(Block orig_block) { // iterate and find prev block
	for (Block block = get_first(); get_next(block).is_valid(); block = get_next(block)) {
		if (get_next(block).get_id() == orig_block.get_id()) {
			return block;
		}
	}
	std::cout << "Warning: get_prev() result is invalid";
	return Block();
}

Block Allocator::get_next(Block orig_block) {
	if (is_last(orig_block) || !orig_block.is_valid()) {
		return Block();
	}
	return Block(orig_block.get_start() + orig_block.get_size());
}

Pointer Allocator::alloc(int N) { // check N
	for (Block block = get_first(); block.is_valid(); block = get_next(block)) {
		// break;
		// block.cout_dump(); // start, size, id, free
		// std::cout << block.is_valid() << std::endl;
		// get_next(block).cout_dump();
		// std::cout << "---" << std::endl;
		if (block.is_free() && block.data_size() >= N) {
			if (block.data_size() == N) {
				block.generate_id();
				Pointer pointer = get_pointer(block);
				return pointer;
			} else {
				int block_start = block.get_start();

				// std::cout << block.get_size() << " prev size" << std::endl;
				block.set_size(block.get_size() - N - Block::meta_size);
				// std::cout << block.get_size() << " size" << std::endl;
				block.move_metadata(block.get_start() + N + Block::meta_size); // move and shrink free block

				Block new_block(block_start, N, false);
				// new_block.cout_dump();
				Pointer pointer = get_pointer(new_block);
				// get_first().cout_dump();
				// get_next(get_first()).cout_dump();
				return pointer;
			}
		}
	}
	throw AllocError(AllocErrorType::NoMemory, "Couldn't allocate a new pointer: out of memory");
}

void Allocator::free(Pointer &p) {
	for (Block block = get_first(); block.is_valid(); block = get_next(block)) {
		if (!block.is_free() && block.get_id() == p.id) {
			block.free();
			p.valid = false;
			merge(block);
			return;
		}
	}
	throw AllocError(AllocErrorType::InvalidFree, "Couldn't free a pointer: invalid address");
}

void Allocator::merge(Block block) {
	if (block.get_start() > 0) {
		Block prev_block = get_prev(block);
		if (prev_block.is_free()) {
			prev_block.set_size(prev_block.get_size() + block.get_size());
			block = prev_block;
		}
	}

	if (!is_last(block)) {
		Block next_block = get_next(block);
		if (next_block.is_free()) {
			block.set_size(block.get_size() + next_block.get_size());
		}
	}
}

void Allocator::cout_dump() {
	std::cout << "memory dump" << std::endl;
	for (Block block = get_first(); block.is_valid(); block = get_next(block)) {
		block.cout_dump();
	}
}

Pointer Allocator::get_pointer(Block block) {
	if (block.is_free()) {
		throw AllocError(AllocErrorType::InvalidFree, "Couldn't get a pointer: invalid address");
	}
	return Pointer(this, block.get_id());
}

Block Allocator::get_block(Pointer pointer) {
	for (Block block = get_first(); block.is_valid(); block = get_next(block)) {
		if (!block.is_free() && block.get_id() == pointer.id) {
			return block;
		}
	}
	throw AllocError(AllocErrorType::InvalidFree, "Couldn't reallocate a pointer: invalid address");
}

void *Allocator::get_memory(int id) {
	// std::cout << "get_memory " << id << std::endl;
	for (Block block = get_first(); block.is_valid(); block = get_next(block)) {
		if (block.get_id() == id) {
			// std::cout << "found " << id << std::endl;
			return (char *) block.get();
		}
	}
	throw AllocError(AllocErrorType::InvalidFree, "Couldn't get a pointer: invalid address");
}

void Allocator::defrag() {
	size_t top = 0;
	for (Block block = get_first(); block.is_valid();) {
		bool view_next = false;
		// block.cout_dump();
		// std::cout << block.is_valid() << std::endl;
		// if (block.is_free()) {
		// 	if (is_last(block)) {
		// 		block.move_metadata(top);
		// 		block.set_size(memsize - top);
		// 	}
		// }
		if (!block.is_free()) {
			// block.cout_dump();
			int block_size = block.get_size();
			if (block.get_start() > top) {
				Block old_block = block;
				block = get_next(block);
				old_block.move_and_free(top);
				view_next = true;
			}
			top += block_size;
			// block.cout_dump();
			// std::cout << "top + " << block.get_size() << " = " << top << std::endl;
		}
		// if (is_last(block)) {
		// 	block.cout_dump();
		// }
		// if (block.is_free() && is_last(block)) {
		// 	block.move_metadata(top);
		// 	block.set_size(memsize - top);
		// }
		if (!view_next) {
			block = get_next(block);
		}
	}
	Block(top, memsize - top - Block::meta_size, true);
}

void Allocator::realloc(Pointer &p, int N) {
	if (!p.valid) {
		p = alloc(N);
		return;
	}

	if (N == 0) {
		free(p);
		return;
	}

	Block block = get_block(p);

	bool prev_included = false;
	bool next_included = false;

	int sum_size = block.data_size();
	if (!is_first(block) && get_prev(block).is_free()) {
		sum_size += get_prev(block).data_size();
		prev_included = true;
	}
	if (!is_last(block) && get_next(block).is_free()) {
		sum_size += get_next(block).data_size();
		next_included = true;
	}

	if (block.data_size() == N) {
		return;
	} else if (block.data_size() > N + Block::meta_size) { // new free block should contain at least 1 byte
		int leftover = block.data_size() - N;
		if (is_last(block) || !get_next(block).is_free()) {
			Block new_free_block = Block(block.get_start() + N + Block::meta_size, leftover, true);
		} else {
			Block next_block = get_next(block);
			next_block.set_size(next_block.get_size() + leftover);
			next_block.move_metadata(block.get_start() + N + Block::meta_size); // next block is free so only move metadata
		}
		block.set_data_size(N);
		return;
	} else {
		if (sum_size > N) { // disregard free space available from shrinking metadata
			// merge block with free neighbors
			if (prev_included) {
				block.move_and_free(get_prev(block).get_start());
				block.set_data_size(N);

				if (next_included) {
					Block(block.get_start() + block.get_size(), sum_size + 2 * Block::meta_size - block.get_size(), true);
				} else {
					Block(block.get_start() + block.get_size(), sum_size + Block::meta_size - block.get_size(), true);
				}
			} else {
				// std::cout << block.get_size() << " " << get_next(block).get_size() << std::endl;
				// std::cout << sum_size << " - " << N << " = " << sum_size - N << std::endl;
				get_next(block).set_data_size(sum_size - N);
				get_next(block).move_metadata(block.get_start() + Block::meta_size + N);
				block.set_data_size(N);
				// std::cout << block.get_size() << " " << get_next(block).get_size() << std::endl;
			}
			return;
		} else {
			// general case
			for (Block free_block = get_first(); free_block.is_valid(); free_block = get_next(free_block)) {
				if (free_block.is_free() && free_block.data_size() > N + Block::meta_size) {
					// new free block should contain at least 1 byte
					// disregard free space available from shrinking metadata
					int leftover = free_block.data_size() - N - Block::meta_size; // full size of new free block
					block.move_and_free(free_block.get_start());
					block.set_data_size(N);

					Block new_free_block = Block(block.get_start() + block.get_size(), leftover, true);

					// merge(new_free_block);
					return;
				}
			}
		}
	}

	throw AllocError(AllocErrorType::NoMemory, "Couldn't reallocate the pointer: out of memory");
}

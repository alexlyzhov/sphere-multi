#include "allocator.h"
#include "allocator_pointer.h"
#include "allocator_error.h"
#include "block.h"
#include <iostream>
#include <cstring>

Allocator::Allocator(void *base, size_t size) : mem(base), memsize(size) {
	blocks.push_back(Block(mem, 0, memsize, true));
}

Pointer Allocator::alloc(size_t N) {
	for(auto it = blocks.begin(); it != blocks.end(); it++) {
		Block &block = *it;
		if (block.is_free() && block.size >= N) {
			if (block.size == N) {
				block.generate_id();
				Pointer pointer = get_pointer(block);
				return pointer;
			} else {
				Block new_block(mem, block.start, N, false);

				block.start += N;
				block.size -= N;

				blocks.insert(it, new_block);
				Pointer pointer = get_pointer(new_block);

				return pointer;
			}
		}
	}
	throw AllocError(AllocErrorType::NoMemory, "Couldn't allocate a new pointer: out of memory");
}

void Allocator::realloc(Pointer& p, size_t N) {
	if (!p.valid) {
		p = alloc(N);
		return;
	}

	if (N == 0) {
		free(p);
		return;
	}

	for(auto it = blocks.begin(); it != blocks.end(); it++) {
		Block &block = *it;
		if (!block.is_free() && block.get_id() == p.id) {
			bool prev_included = false;
			bool next_included = false;

			int sum_size = block.size;
			if (it != blocks.begin() && std::prev(it)->is_free()) {
				sum_size += std::prev(it)->size;
				prev_included = true;
			}
			if (std::next(it) != blocks.end() && std::next(it)->is_free()) {
				sum_size += std::next(it)->size;
				next_included = true;
			}

			if (block.size == N) {
				return;
			} else if (block.size > N) {
				size_t free_size = block.size - N;
				block.size = N;
				if (std::next(it) == blocks.end() || !std::next(it)->is_free()) {
					blocks.insert(std::next(it), Block(mem, block.start + N, free_size, true));
				} else {
					std::next(it)->start -= free_size;
					std::next(it)->size += free_size;
				}
				return;
			} else {
				if (sum_size >= N) {
					// merge block with free neighbors
					auto prev_it = std::prev(it);
					auto next_it = std::next(it);
					if (prev_included) {
						memcpy(prev_it->get(), block.get(), block.size);

						prev_it->set_id(block.get_id());
						if (prev_it->size + block.size + next_it->size == N) {
							prev_it->size = N;
							blocks.erase(std::next(it));
							blocks.erase(it);
						} else if (prev_it->size + block.size == N) {
							prev_it->size = N;
							blocks.erase(it);
						} else {
							prev_it->size = N;

							block.start = prev_it->start + N;
							block.size = sum_size - prev_it->size;

							if (next_included) {
								blocks.erase(std::next(it));
							}
						}
					} else {
						if (block.size + next_it->size != N) {
							block.size = N;
							next_it->start = block.start + N;
							next_it->size = sum_size - N;
							next_it->free();
						} else {
							block.size = N;
							blocks.erase(next_it);
						}
					}
					return;
				} else {
					// general case
					for (auto nit = blocks.begin(); nit != blocks.end(); nit++) {
						Block &free_block = *nit;
						if (free_block.is_free() && free_block.size >= N) {
							size_t leftover = free_block.size - N;
							memcpy(free_block.get(), block.get(), block.size);
							free_block.set_id(block.get_id());
							free_block.size = N;

							block.free();

							if (leftover > 0) {
								blocks.insert(nit+1, Block(mem, free_block.start + N, leftover, true));
							}

							merge(it);
							return;
						}
					}
				}
			}

			throw AllocError(AllocErrorType::NoMemory, "Couldn't reallocate the pointer: out of memory");
		}
	}
	throw AllocError(AllocErrorType::InvalidFree, "Couldn't reallocate a pointer: invalid address");
}

void Allocator::free(Pointer &p) {
	for (std::vector<Block>::iterator it = blocks.begin(); it != blocks.end(); it++) {
		Block &block = *it;
		if (!block.is_free() && block.get_id() == p.id) {
			block.free();
			p.valid = false;
			merge(it);			
			return;
		}
	}
	throw AllocError(AllocErrorType::InvalidFree, "Couldn't free a pointer: invalid address");
}

void Allocator::merge(std::vector<Block>::iterator it) {
	Block &block = *it;

	if (block.start > 0) {
		Block &prev_block = *std::prev(it);
		if (prev_block.is_free()) {
			block.start = prev_block.start;
			block.size = prev_block.size + block.size;
			it = blocks.erase(std::prev(it));
		}
	}

	Block &new_block = *it;

	if (new_block.start + new_block.size < memsize) {
		Block &next_block = *std::next(it);
		if (next_block.is_free()) {
			new_block.size = new_block.size + next_block.size;
			blocks.erase(std::next(it));
		}
	}
}

void Allocator::defrag() {
	size_t top = 0;
	for (std::vector<Block>::iterator it = blocks.begin(); it != blocks.end(); it++) {
		Block &block = *it;
		if (block.is_free()) {
			if (it+1 == blocks.end()) {
				block.start = top;
				block.size = memsize - top;
			} else {
				it = blocks.erase(it);
				it--;
			}
		} else {
			if (block.start > top) {
				memcpy((char *) mem + top, block.get(), block.size);
				block.start = top;
			}
			top += block.size;
		}
	}
}

void Allocator::cout_dump() {
	std::cout << "memory dump" << std::endl;
	for (auto it = blocks.begin(); it != blocks.end(); it++) {
		Block block = *it;
		std::cout << block.start << ", " << block.size << ", " << block.get_id() << ", " << block.is_free() << std::endl;
	}
}

Pointer Allocator::get_pointer(Block block) {
	if (block.is_free()) {
		throw AllocError(AllocErrorType::InvalidFree, "Couldn't get a pointer: invalid address");
	}
	return Pointer(this, block.get_id());
}

void *Allocator::get_memory(int id) {
	for (auto it = blocks.begin(); it != blocks.end(); it++) {
		Block block = *it;
		if (block.get_id() == id) {
			return (char *) mem + block.start;
		}
	}
	throw AllocError(AllocErrorType::InvalidFree, "Couldn't get a pointer: invalid address");
}

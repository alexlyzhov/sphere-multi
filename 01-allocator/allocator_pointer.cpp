#include "allocator_pointer.h"
#include "allocator.h"

Pointer::Pointer(Allocator *allocator, int id) : allocator(allocator), id(id), valid(true) {
	
}

Pointer::Pointer() : valid(false) {

}

void *Pointer::get() const {
	if (!valid) {
		return nullptr;
	} else {
		return allocator->get_memory(id);
	}
}

bool Pointer::operator==(const Pointer &another) const {
    return (id == another.id);
}
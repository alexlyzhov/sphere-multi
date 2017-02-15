#ifndef POINTER
#define POINTER

#include <cstdlib>

// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Allocator;

class Pointer {
public:
	Allocator *allocator;
	int id;
	bool valid;

	Pointer();
	Pointer(Allocator *allocator, int id);
    void* get() const;
    bool operator==(const Pointer &another) const;
};

#endif //POINTER

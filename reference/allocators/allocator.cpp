#include "allocators/allocator.h"

namespace alloc {

int allocator_is_valid(const Allocator& allocator) {
    return allocator.allocate != 0 && allocator.release != 0;
}

void* allocator_allocate(Allocator& allocator, size_t size, size_t alignment) {
    if (allocator.allocate == 0 || size == 0u) {
        return 0;
    }

    return allocator.allocate(allocator.context, size, alignment);
}

void allocator_release(Allocator& allocator, void* memory, size_t size, size_t alignment) {
    if (allocator.release == 0 || memory == 0) {
        return;
    }

    allocator.release(allocator.context, memory, size, alignment);
}

} // namespace alloc

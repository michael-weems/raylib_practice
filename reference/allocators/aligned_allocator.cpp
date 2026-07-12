#include "allocators/aligned_allocator.h"

#include <stdlib.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

namespace alloc {

static int alignment_is_valid(size_t alignment) {
    return alignment >= sizeof(void*) && (alignment & (alignment - 1u)) == 0u;
}

static void* aligned_allocate(void*, size_t size, size_t alignment) {
    if (size == 0u || !alignment_is_valid(alignment)) {
        return 0;
    }

#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#else
    void* memory = 0;
    if (posix_memalign(&memory, alignment, size) != 0) {
        return 0;
    }

    return memory;
#endif
}

static void aligned_release(void*, void* memory, size_t, size_t) {
    if (memory == 0) {
        return;
    }

#if defined(_MSC_VER)
    _aligned_free(memory);
#else
    free(memory);
#endif
}

Allocator aligned_allocator_create(void) {
    Allocator allocator = {};
    allocator.allocate = aligned_allocate;
    allocator.release = aligned_release;
    return allocator;
}

} // namespace alloc

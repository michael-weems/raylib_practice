#pragma once

#include "allocators/allocator.h"

#include <stddef.h>
#include <stdint.h>

namespace alloc {

struct Arena_Allocator {
    uint8_t* memory;
    size_t capacity;
    size_t used;
    size_t high_water_mark;
};

// Binds caller-owned memory. The arena never grows or falls back to the heap;
// reset invalidates all allocations while retaining the high-water mark.
int arena_allocator_initialize(Arena_Allocator& arena, void* memory, size_t capacity);
void arena_allocator_reset(Arena_Allocator& arena);
// Rejects zero size, invalid alignment, overflow, and exhaustion with null.
void* arena_allocator_allocate(Arena_Allocator& arena, size_t size, size_t alignment);
size_t arena_allocator_remaining(const Arena_Allocator& arena);
// Individual release calls are no-ops; reset reclaims the complete arena.
Allocator arena_allocator_create_interface(Arena_Allocator& arena);

} // namespace alloc

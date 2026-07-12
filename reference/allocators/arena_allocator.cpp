#include "allocators/arena_allocator.h"

#include <string.h>

namespace alloc {

static int alignment_is_valid(size_t alignment) {
    return alignment != 0u && (alignment & (alignment - 1u)) == 0u;
}

int arena_allocator_initialize(Arena_Allocator& arena, void* memory, size_t capacity) {
    memset(&arena, 0, sizeof(arena));
    if (memory == 0 || capacity == 0u) {
        return 0;
    }

    arena.memory = (uint8_t*)memory;
    arena.capacity = capacity;
    return 1;
}

void arena_allocator_reset(Arena_Allocator& arena) {
    arena.used = 0u;
}

void* arena_allocator_allocate(Arena_Allocator& arena, size_t size, size_t alignment) {
    if (arena.memory == 0 || arena.used > arena.capacity || size == 0u ||
        !alignment_is_valid(alignment)) {
        return 0;
    }

    uintptr_t memory_address = (uintptr_t)arena.memory;
    if (memory_address > UINTPTR_MAX - arena.used) {
        return 0;
    }

    uintptr_t current_address = memory_address + arena.used;
    uintptr_t alignment_mask = (uintptr_t)alignment - 1u;
    if (current_address > UINTPTR_MAX - alignment_mask) {
        return 0;
    }

    uintptr_t aligned_address = (current_address + alignment_mask) & ~alignment_mask;
    if (aligned_address < current_address) {
        return 0;
    }

    size_t padding = (size_t)(aligned_address - current_address);
    size_t remaining = arena.capacity - arena.used;
    if (padding > remaining || size > remaining - padding) {
        return 0;
    }

    arena.used += padding + size;
    if (arena.used > arena.high_water_mark) {
        arena.high_water_mark = arena.used;
    }

    return (void*)aligned_address;
}

size_t arena_allocator_remaining(const Arena_Allocator& arena) {
    if (arena.used > arena.capacity) {
        return 0u;
    }

    return arena.capacity - arena.used;
}

static void* arena_allocate_callback(void* context, size_t size, size_t alignment) {
    if (context == 0) {
        return 0;
    }

    return arena_allocator_allocate(*(Arena_Allocator*)context, size, alignment);
}

static void arena_release_callback(void*, void*, size_t, size_t) {}

Allocator arena_allocator_create_interface(Arena_Allocator& arena) {
    Allocator allocator = {};
    allocator.context = &arena;
    allocator.allocate = arena_allocate_callback;
    allocator.release = arena_release_callback;
    return allocator;
}

} // namespace alloc

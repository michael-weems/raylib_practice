#ifndef REFERENCE_ALLOCATORS_ARENA_ALLOCATOR_H
#define REFERENCE_ALLOCATORS_ARENA_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

namespace allocators {

typedef void* (*Allocator_Reserve_Fn)(void* user, size_t size, size_t alignment);
typedef void  (*Allocator_Release_Fn)(void* user, void* ptr);

// Small allocator interface used by project APIs that need memory.
//
// The application/composition layer chooses which concrete allocator backs this
// interface, so lower-level modules never secretly choose a layout policy.
struct Allocator {
    void* user;
    Allocator_Reserve_Fn reserve;
    Allocator_Release_Fn release;
};

// Bump allocator for transient per-frame data. It owns no memory: the caller
// supplies a buffer, work is pushed linearly, and the whole arena is reset at
// the end of each frame.
struct Arena_Allocator {
    uint8_t* memory;
    size_t capacity;
    size_t used;
};

// Simple persistent allocator backed by aligned malloc/free. It is intentionally
// tiny; it exists so APIs can still accept Allocator& without dragging in STL.
struct Malloc_Allocator {
    int unused;
};

void* allocator_reserve(Allocator& allocator, size_t size, size_t alignment);
void allocator_release(Allocator& allocator, void* ptr);

void arena_init(Arena_Allocator& arena, void* memory, size_t capacity);
void arena_reset(Arena_Allocator& arena);
void* arena_push(Arena_Allocator& arena, size_t size, size_t alignment);
Allocator arena_as_allocator(Arena_Allocator& arena);

Allocator malloc_allocator(void);

} // namespace allocators

#endif // REFERENCE_ALLOCATORS_ARENA_ALLOCATOR_H

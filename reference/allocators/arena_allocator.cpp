#include "allocators/arena_allocator.h"

#include <stdint.h>
#include <stdlib.h>

namespace alloc {

static uintptr_t align_forward(uintptr_t value, size_t align)
{
    uintptr_t mask = 0;
    if (align) {
        mask = (uintptr_t)(align - 1u);
    }
    return (value + mask) & ~mask;
}

static void* malloc_alloc(void*, size_t size, size_t align)
{
    size_t header = sizeof(void*) + align;
    void* raw = malloc(size + header);
    if (!raw) {
        return 0;
    }

    size_t effective_align = align;
    if (!effective_align) {
        effective_align = sizeof(void*);
    }

    uintptr_t start = (uintptr_t)raw + sizeof(void*);
    uintptr_t aligned = align_forward(start, effective_align);
    ((void**)aligned)[-1] = raw;
    return (void*)aligned;
}

static void malloc_free(void*, void* ptr)
{
    if (ptr) {
        free(((void**)ptr)[-1]);
    }
}

static void* arena_alloc_cb(void* user, size_t size, size_t align)
{
    return arena_push(*(Arena*)user, size, align);
}

static void arena_free_cb(void*, void*) {}

Allocator malloc_allocator(void)
{
    Allocator a = {};
    a.alloc = malloc_alloc;
    a.free = malloc_free;
    return a;
}

void* allocator_alloc(Allocator& allocator, size_t size, size_t align)
{
    if (!allocator.alloc) {
        return 0;
    }

    return allocator.alloc(allocator.user, size, align);
}

void allocator_free(Allocator& allocator, void* ptr)
{
    if (allocator.free) {
        allocator.free(allocator.user, ptr);
    }
}

void arena_init(Arena& arena, void* memory, size_t capacity)
{
    arena.memory = (unsigned char*)memory;
    arena.capacity = capacity;
    arena.used = 0;
}

void arena_reset(Arena& arena)
{
    arena.used = 0;
}

void* arena_push(Arena& arena, size_t size, size_t align)
{
    size_t effective_align = align;
    if (!effective_align) {
        effective_align = sizeof(void*);
    }

    uintptr_t base = (uintptr_t)arena.memory;
    uintptr_t at = align_forward(base + arena.used, effective_align);
    size_t next = (size_t)(at - base) + size;
    if (!arena.memory || next > arena.capacity) {
        return 0;
    }

    arena.used = next;
    return (void*)at;
}

Allocator arena_allocator(Arena& arena)
{
    Allocator a = {};
    a.user = &arena;
    a.alloc = arena_alloc_cb;
    a.free = arena_free_cb;
    return a;
}

}

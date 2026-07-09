#include "allocators/arena_allocator.h"

#include <stdlib.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

namespace allocators {

static size_t align_forward(size_t value, size_t alignment)
{
    if (alignment == 0) {
        alignment = 1;
    }

    size_t mask = alignment - 1;
    return (value + mask) & ~mask;
}

void* allocator_reserve(Allocator& allocator, size_t size, size_t alignment)
{
    if (!allocator.reserve) {
        return 0;
    }

    return allocator.reserve(allocator.user, size, alignment);
}

void allocator_release(Allocator& allocator, void* ptr)
{
    if (allocator.release && ptr) {
        allocator.release(allocator.user, ptr);
    }
}

void arena_init(Arena_Allocator& arena, void* memory, size_t capacity)
{
    arena.memory = (uint8_t*)memory;
    arena.capacity = capacity;
    arena.used = 0;
}

void arena_reset(Arena_Allocator& arena)
{
    arena.used = 0;
}

void* arena_push(Arena_Allocator& arena, size_t size, size_t alignment)
{
    size_t aligned_used = align_forward(arena.used, alignment);
    size_t next_used = aligned_used + size;

    if (!arena.memory || next_used > arena.capacity) {
        return 0;
    }

    void* result = arena.memory + aligned_used;
    arena.used = next_used;
    return result;
}

static void* arena_allocator_reserve(void* user, size_t size, size_t alignment)
{
    Arena_Allocator* arena = (Arena_Allocator*)user;
    return arena_push(*arena, size, alignment);
}

static void arena_allocator_release(void* user, void* ptr)
{
    (void)user;
    (void)ptr;
}

Allocator arena_as_allocator(Arena_Allocator& arena)
{
    Allocator allocator = {};
    allocator.user = &arena;
    allocator.reserve = arena_allocator_reserve;
    allocator.release = arena_allocator_release;
    return allocator;
}

static void* malloc_allocator_reserve(void* user, size_t size, size_t alignment)
{
    (void)user;

    if (alignment < sizeof(void*)) {
        alignment = sizeof(void*);
    }

#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#else
    void* ptr = 0;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return 0;
    }
    return ptr;
#endif
}

static void malloc_allocator_release(void* user, void* ptr)
{
    (void)user;

#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

Allocator malloc_allocator(void)
{
    Allocator allocator = {};
    allocator.user = 0;
    allocator.reserve = malloc_allocator_reserve;
    allocator.release = malloc_allocator_release;
    return allocator;
}

} // namespace allocators

#pragma once

#include <stddef.h>

namespace alloc {

typedef void* (*Allocator_Alloc_Fn)(void* user, size_t size, size_t align);
typedef void  (*Allocator_Free_Fn)(void* user, void* ptr);

struct Allocator {
    void* user;
    Allocator_Alloc_Fn alloc;
    Allocator_Free_Fn free;
};

struct Arena {
    unsigned char* memory;
    size_t capacity;
    size_t used;
};

Allocator malloc_allocator(void);
void* allocator_alloc(Allocator& allocator, size_t size, size_t align);
void allocator_free(Allocator& allocator, void* ptr);

void arena_init(Arena& arena, void* memory, size_t capacity);
void arena_reset(Arena& arena);
void* arena_push(Arena& arena, size_t size, size_t align);
Allocator arena_allocator(Arena& arena);

}

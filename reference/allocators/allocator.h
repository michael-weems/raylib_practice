#pragma once

#include <stddef.h>

namespace alloc {

typedef void* (*Allocator_Allocate_Function)(void* context, size_t size, size_t alignment);
// Size and alignment repeat on release so pools need no hidden headers.
typedef void (*Allocator_Release_Function)(void* context, void* memory, size_t size,
                                           size_t alignment);

struct Allocator {
    void* context;
    Allocator_Allocate_Function allocate;
    Allocator_Release_Function release;
};

// Operations publish temporary-storage needs without choosing placement.
struct Allocator_Requirements {
    size_t capacity;
    size_t alignment;
};

// The all-zero allocator is a safe inert stub. Owning APIs accept the caller's
// allocator so the application controls placement and lifetime.
int allocator_is_valid(const Allocator& allocator);
// Zero size or a missing callback returns null without dispatching.
void* allocator_allocate(Allocator& allocator, size_t size, size_t alignment);
// Null memory or a missing callback is a safe no-op.
void allocator_release(Allocator& allocator, void* memory, size_t size, size_t alignment);

} // namespace alloc

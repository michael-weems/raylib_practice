#pragma once

#include "allocators/allocator.h"

namespace alloc {

enum { ALLOCATOR_CACHE_LINE_SIZE = 64 };

// Stateless persistent heap policy used by the reference composition root.
Allocator aligned_allocator_create(void);

} // namespace alloc

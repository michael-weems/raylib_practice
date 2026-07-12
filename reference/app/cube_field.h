#pragma once

#include "allocators/allocator.h"
#include "raylib.h"

#include <stddef.h>
#include <stdint.h>

namespace app {

typedef uint32_t Cube_Handle;
typedef uint32_t Cube_Palette_Handle;

enum {
    CUBE_HANDLE_NONE = 0,
    CUBE_HANDLE_FIRST = 1,
    CUBE_PALETTE_HANDLE_NONE = 0,
    CUBE_PALETTE_HANDLE_DEFAULT = 1,
    CUBE_PALETTE_HANDLE_PASTEL = 2,
    CUBE_PALETTE_HANDLE_WARM = 3,
    CUBE_PALETTE_COUNT = 3,
    CUBE_PALETTE_CAPACITY = CUBE_PALETTE_COUNT + 1
};

enum Cube_Value : uint8_t {
    CUBE_VALUE_NONE = 0,
    CUBE_VALUE_A,
    CUBE_VALUE_B,
    CUBE_VALUE_C,
    CUBE_VALUE_D,
    CUBE_VALUE_COUNT
};

struct Grid_Dimensions {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct Grid_Coordinate {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct Cube_Field_Config {
    Grid_Dimensions dimensions;
    float cube_size;
    float spacing;
};

struct Cube_Field {
    // X is contiguous, Y advances stride_y, and Z advances stride_z.
    Grid_Dimensions dimensions;
    uint32_t stride_y;
    uint32_t stride_z;
    uint32_t cube_count;
    float cube_size;
    float spacing;
    Vector3 first_center;
    Vector3 bounds_min;
    Vector3 bounds_max;
};

// One zero stub byte precedes x-contiguous immutable A/B/C/D values. Handles
// index this stream directly and never expose pointer identity.
struct Cube_Data {
    uint8_t* values;
    size_t allocation_size;
    size_t allocation_alignment;
    uint32_t cube_count;
    uint8_t zero_stub;
    uint8_t reserved[3];
};

// Fill and edge colors are consumed together in render hot loops.
struct Cube_Visual_Style {
    Color fill_color;
    Color edge_color;
};

struct Cube_Palette {
    Cube_Visual_Style styles[CUBE_VALUE_COUNT];
    const char* name;
};

struct Cube_Palette_Set {
    Cube_Palette palettes[CUBE_PALETTE_CAPACITY];
    Cube_Palette_Handle active_palette;
};

// Initialization checks the complete one-based handle range and derives all
// transforms. Cube positions are never stored per cube.
int cube_field_initialize(Cube_Field& field, const Cube_Field_Config& config);
int cube_field_contains(const Cube_Field& field, Grid_Coordinate coordinate);
// In-bounds coordinates map to [1, cube_count]; invalid coordinates map to 0.
Cube_Handle cube_field_handle(const Cube_Field& field, Grid_Coordinate coordinate);
int cube_field_contains_handle(const Cube_Field& field, Cube_Handle cube);
// Handle zero and invalid handles return the all-zero coordinate stub.
Grid_Coordinate cube_field_coordinate(const Cube_Field& field, Cube_Handle cube);
Vector3 cube_field_coordinate_center(const Cube_Field& field, Grid_Coordinate coordinate);
Vector3 cube_field_cube_center(const Cube_Field& field, Cube_Handle cube);
Vector3 cube_field_center(const Cube_Field& field);
Vector3 cube_field_half_extents(const Cube_Field& field);
void cube_field_cube_bounds(const Cube_Field& field, Cube_Handle cube, Vector3& bounds_min,
                            Vector3& bounds_max);
// Converts a point in field bounds to its nearest lattice coordinate and handle.
Cube_Handle cube_field_world_handle(const Cube_Field& field, Vector3 world_position);

// Allocates count+1 bytes through caller policy, writes slot zero, then
// deterministically generates immutable A/B/C/D values in x-fastest order.
int cube_data_initialize(Cube_Data& data, const Cube_Field& field, alloc::Allocator& allocator);
// Releases with the original size/alignment and restores the zero state.
void cube_data_shutdown(Cube_Data& data, alloc::Allocator& allocator);
// Zero, invalid, and zero-state lookups return CUBE_VALUE_NONE.
uint8_t cube_data_value(const Cube_Data& data, Cube_Handle cube);

// Slot zero remains an all-zero visual stub; real palettes begin at handle one.
void cube_palette_set_initialize(Cube_Palette_Set& set);
// Invalid selection intentionally selects the zero palette stub.
void cube_palette_set_select(Cube_Palette_Set& set, Cube_Palette_Handle palette);
// Zero and invalid handles return the inline all-zero palette by reference.
const Cube_Palette& cube_palette_set_get(const Cube_Palette_Set& set, Cube_Palette_Handle palette);
const Cube_Palette& cube_palette_set_active(const Cube_Palette_Set& set);
// Zero and invalid values return the palette's all-zero style by reference.
const Cube_Visual_Style& cube_palette_style(const Cube_Palette& palette, uint8_t value);
// Zero and invalid values return a stable empty string.
const char* cube_value_label(uint8_t value);

} // namespace app

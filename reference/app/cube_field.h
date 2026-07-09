#pragma once

#include "allocators/arena_allocator.h"
#include "raylib.h"

#include <stdint.h>

namespace app {

typedef uint32_t Cube_Handle;

enum Cube_Value {
    CUBE_VALUE_NONE = 0,
    CUBE_VALUE_A = 1,
    CUBE_VALUE_B = 2,
    CUBE_VALUE_C = 3,
    CUBE_VALUE_D = 4,
    CUBE_VALUE_COUNT = 5
};

enum { CUBE_HANDLE_STUB = 0, CUBE_HANDLE_FIRST = 1 };

struct Cube_Coords {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct Cube_Field {
    uint32_t count_x;
    uint32_t count_y;
    uint32_t count_z;
    uint32_t total_count;
    float cube_size;
    float spacing;
    Vector3 first_center;
    Vector3 bounds_min;
    Vector3 bounds_max;
};

struct Cube_Data {
    uint8_t* values;
    uint32_t count;
};

struct Cube_Palette {
    Color fill[CUBE_VALUE_COUNT];
    Color edge[CUBE_VALUE_COUNT];
    const char* name;
};

struct Cube_Palette_Table {
    Cube_Palette palettes[3];
    uint32_t count;
};

void cube_field_init(Cube_Field& field, uint32_t count_x, uint32_t count_y, uint32_t count_z, float cube_size, float spacing);
Cube_Handle cube_handle_from_coords(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z);
Cube_Coords cube_coords_from_handle(const Cube_Field& field, Cube_Handle handle);
Cube_Handle cube_handle_from_world(const Cube_Field& field, Vector3 world);
Vector3 cube_center(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z);
void cube_bounds(const Cube_Field& field, Cube_Handle handle, Vector3& out_min, Vector3& out_max);

int cube_data_generate(Cube_Data& data, const Cube_Field& field, alloc::Allocator& allocator);
void cube_data_release(Cube_Data& data, alloc::Allocator& allocator);
Cube_Value cube_value_at(const Cube_Data& data, Cube_Handle handle);
const char* cube_value_name(Cube_Value value);

void cube_palettes_init(Cube_Palette_Table& table);
const Cube_Palette& cube_palette(const Cube_Palette_Table& table, uint32_t index);

}

#ifndef REFERENCE_APP_CUBE_FIELD_H
#define REFERENCE_APP_CUBE_FIELD_H

#include "allocators/arena_allocator.h"

#include "raylib.h"

#include <stdint.h>

namespace app {

typedef uint32_t Cube_Handle;

enum Cube_Value {
    CUBE_VALUE_NONE = 0,
    CUBE_VALUE_A,
    CUBE_VALUE_B,
    CUBE_VALUE_C,
    CUBE_VALUE_D,
    CUBE_VALUE_COUNT
};

struct Cube_Coords {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct Cube_Field {
    uint32_t count_x;
    uint32_t count_y;
    uint32_t count_z;
    uint64_t total_count;

    float cube_size;
    float spacing;
};

struct Cube_Data {
    uint8_t* values;
    uint64_t value_count;
};

struct Cube_Palette {
    Color fill[CUBE_VALUE_COUNT];
    Color edge[CUBE_VALUE_COUNT];
};

enum Cube_Data_Result {
    CUBE_DATA_SUCCESS = 0,
    CUBE_DATA_ERROR_INVALID_FIELD,
    CUBE_DATA_ERROR_ALLOCATION_FAILED
};

void cube_field_init(Cube_Field& field, uint32_t count_x, uint32_t count_y, uint32_t count_z, float cube_size, float spacing);

Cube_Data_Result cube_data_create(
    allocators::Allocator& allocator,
    const Cube_Field& field,
    Cube_Data& data);

void cube_data_destroy(allocators::Allocator& allocator, Cube_Data& data);
void cube_data_generate(const Cube_Field& field, Cube_Data& data, uint32_t seed);

void cube_palette_default(Cube_Palette& palette);
void cube_palette_variant(Cube_Palette& palette, uint32_t palette_index);

Cube_Handle cube_handle_from_coords(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z);
Cube_Coords cube_coords_from_handle(const Cube_Field& field, Cube_Handle handle);
Cube_Handle cube_handle_clamped(const Cube_Field& field, int x, int y, int z);

Cube_Value cube_value_at(const Cube_Data& data, Cube_Handle handle);
const char* cube_value_name(Cube_Value value);

Vector3 cube_center(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z);
Vector3 cube_center_from_handle(const Cube_Field& field, Cube_Handle handle);
BoundingBox cube_bounds(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z);

Vector3 cube_field_min(const Cube_Field& field);
Vector3 cube_field_max(const Cube_Field& field);
Vector3 cube_field_center(const Cube_Field& field);
Vector3 cube_field_world_size(const Cube_Field& field);

} // namespace app

#endif // REFERENCE_APP_CUBE_FIELD_H

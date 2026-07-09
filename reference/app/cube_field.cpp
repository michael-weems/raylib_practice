#include "app/cube_field.h"

#include <stddef.h>

namespace app {

static uint32_t hash_u32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

void cube_field_init(Cube_Field& field, uint32_t count_x, uint32_t count_y, uint32_t count_z, float cube_size, float spacing)
{
    field.count_x = count_x;
    field.count_y = count_y;
    field.count_z = count_z;
    field.total_count = (uint64_t)count_x * (uint64_t)count_y * (uint64_t)count_z;
    field.cube_size = cube_size;
    field.spacing = spacing;
}

Cube_Data_Result cube_data_create(
    allocators::Allocator& allocator,
    const Cube_Field& field,
    Cube_Data& data)
{
    if (field.count_x == 0 || field.count_y == 0 || field.count_z == 0 || field.total_count == 0) {
        return CUBE_DATA_ERROR_INVALID_FIELD;
    }

    uint64_t needed_count = field.total_count + 1u;
    data.values = (uint8_t*)allocators::allocator_reserve(allocator, (size_t)needed_count * sizeof(uint8_t), 64);
    data.value_count = needed_count;

    if (!data.values) {
        data.value_count = 0;
        return CUBE_DATA_ERROR_ALLOCATION_FAILED;
    }

    for (uint64_t i = 0; i < data.value_count; ++i) {
        data.values[i] = 0;
    }

    return CUBE_DATA_SUCCESS;
}

void cube_data_destroy(allocators::Allocator& allocator, Cube_Data& data)
{
    allocators::allocator_release(allocator, data.values);
    data.values = 0;
    data.value_count = 0;
}

void cube_data_generate(const Cube_Field& field, Cube_Data& data, uint32_t seed)
{
    if (!data.values) {
        return;
    }

    data.values[0] = CUBE_VALUE_NONE;

    for (uint32_t z = 0; z < field.count_z; ++z) {
        for (uint32_t y = 0; y < field.count_y; ++y) {
            for (uint32_t x = 0; x < field.count_x; ++x) {
                Cube_Handle handle = cube_handle_from_coords(field, x, y, z);
                uint32_t h = hash_u32(seed ^ (x * 0x9e3779b9u) ^ (y * 0x85ebca6bu) ^ (z * 0xc2b2ae35u));
                data.values[handle] = (uint8_t)(CUBE_VALUE_A + (h & 3u));
            }
        }
    }
}

void cube_palette_default(Cube_Palette& palette)
{
    for (int i = 0; i < CUBE_VALUE_COUNT; ++i) {
        palette.fill[i] = BLANK;
        palette.edge[i] = GRAY;
    }

    palette.fill[CUBE_VALUE_A] = Color{  35, 110, 255, 255 };
    palette.fill[CUBE_VALUE_B] = Color{ 225,  45,  55, 255 };
    palette.fill[CUBE_VALUE_C] = Color{  40, 200,  90, 255 };
    palette.fill[CUBE_VALUE_D] = Color{ 245, 245, 245, 153 };

    palette.edge[CUBE_VALUE_A] = Color{ 120, 180, 255, 255 };
    palette.edge[CUBE_VALUE_B] = Color{ 255, 135, 135, 255 };
    palette.edge[CUBE_VALUE_C] = Color{ 135, 255, 170, 255 };
    palette.edge[CUBE_VALUE_D] = Color{ 255, 255, 255, 255 };
}

void cube_palette_variant(Cube_Palette& palette, uint32_t palette_index)
{
    cube_palette_default(palette);

    if (palette_index == 2u) {
        palette.fill[CUBE_VALUE_A] = Color{  80, 180, 255, 255 };
        palette.fill[CUBE_VALUE_B] = Color{ 255, 135,  40, 255 };
        palette.fill[CUBE_VALUE_C] = Color{ 210,  70, 255, 255 };
        palette.fill[CUBE_VALUE_D] = Color{ 245, 245, 245, 153 };

        palette.edge[CUBE_VALUE_A] = Color{ 180, 225, 255, 255 };
        palette.edge[CUBE_VALUE_B] = Color{ 255, 210, 130, 255 };
        palette.edge[CUBE_VALUE_C] = Color{ 245, 175, 255, 255 };
        palette.edge[CUBE_VALUE_D] = Color{ 255, 255, 255, 255 };
    } else if (palette_index == 3u) {
        palette.fill[CUBE_VALUE_A] = Color{  20,  60, 180, 255 };
        palette.fill[CUBE_VALUE_B] = Color{ 160,  30,  30, 255 };
        palette.fill[CUBE_VALUE_C] = Color{  25, 135,  70, 255 };
        palette.fill[CUBE_VALUE_D] = Color{ 220, 220, 220, 153 };

        palette.edge[CUBE_VALUE_A] = Color{ 110, 145, 255, 255 };
        palette.edge[CUBE_VALUE_B] = Color{ 230, 100, 100, 255 };
        palette.edge[CUBE_VALUE_C] = Color{ 110, 220, 150, 255 };
        palette.edge[CUBE_VALUE_D] = Color{ 255, 255, 255, 255 };
    }
}

Cube_Handle cube_handle_from_coords(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z)
{
    if (x >= field.count_x || y >= field.count_y || z >= field.count_z) {
        return 0;
    }

    uint64_t zero_based = (uint64_t)x + (uint64_t)field.count_x * ((uint64_t)y + (uint64_t)field.count_y * (uint64_t)z);
    return (Cube_Handle)(zero_based + 1u);
}

Cube_Coords cube_coords_from_handle(const Cube_Field& field, Cube_Handle handle)
{
    Cube_Coords coords = {};

    if (handle == 0) {
        return coords;
    }

    uint64_t zero_based = (uint64_t)handle - 1u;
    uint64_t xy_count = (uint64_t)field.count_x * (uint64_t)field.count_y;

    coords.z = (uint32_t)(zero_based / xy_count);
    uint64_t remainder = zero_based - (uint64_t)coords.z * xy_count;
    coords.y = (uint32_t)(remainder / field.count_x);
    coords.x = (uint32_t)(remainder - (uint64_t)coords.y * field.count_x);

    return coords;
}

Cube_Handle cube_handle_clamped(const Cube_Field& field, int x, int y, int z)
{
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (z < 0) {
        z = 0;
    }

    if (x >= (int)field.count_x) {
        x = (int)field.count_x - 1;
    }
    if (y >= (int)field.count_y) {
        y = (int)field.count_y - 1;
    }
    if (z >= (int)field.count_z) {
        z = (int)field.count_z - 1;
    }

    return cube_handle_from_coords(field, (uint32_t)x, (uint32_t)y, (uint32_t)z);
}

Cube_Value cube_value_at(const Cube_Data& data, Cube_Handle handle)
{
    if (!data.values || handle >= data.value_count) {
        return CUBE_VALUE_NONE;
    }

    return (Cube_Value)data.values[handle];
}

const char* cube_value_name(Cube_Value value)
{
    switch (value) {
        case CUBE_VALUE_A: return "A";
        case CUBE_VALUE_B: return "B";
        case CUBE_VALUE_C: return "C";
        case CUBE_VALUE_D: return "D";
        default: break;
    }

    return "NONE";
}

Vector3 cube_center(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z)
{
    Vector3 result = {};
    result.x = field.spacing * ((float)x - ((float)field.count_x - 1.0f) * 0.5f);
    result.y = field.spacing * ((float)y - ((float)field.count_y - 1.0f) * 0.5f);
    result.z = field.spacing * ((float)z - ((float)field.count_z - 1.0f) * 0.5f);
    return result;
}

Vector3 cube_center_from_handle(const Cube_Field& field, Cube_Handle handle)
{
    Cube_Coords coords = cube_coords_from_handle(field, handle);
    return cube_center(field, coords.x, coords.y, coords.z);
}

BoundingBox cube_bounds(const Cube_Field& field, uint32_t x, uint32_t y, uint32_t z)
{
    Vector3 center = cube_center(field, x, y, z);
    float half = field.cube_size * 0.5f;

    BoundingBox box = {};
    box.min = Vector3{ center.x - half, center.y - half, center.z - half };
    box.max = Vector3{ center.x + half, center.y + half, center.z + half };
    return box;
}

Vector3 cube_field_min(const Cube_Field& field)
{
    Vector3 first = cube_center(field, 0, 0, 0);
    float half = field.cube_size * 0.5f;
    return Vector3{ first.x - half, first.y - half, first.z - half };
}

Vector3 cube_field_max(const Cube_Field& field)
{
    Vector3 last = cube_center(field, field.count_x - 1u, field.count_y - 1u, field.count_z - 1u);
    float half = field.cube_size * 0.5f;
    return Vector3{ last.x + half, last.y + half, last.z + half };
}

Vector3 cube_field_center(const Cube_Field& field)
{
    (void)field;
    return Vector3{ 0.0f, 0.0f, 0.0f };
}

Vector3 cube_field_world_size(const Cube_Field& field)
{
    Vector3 min = cube_field_min(field);
    Vector3 max = cube_field_max(field);
    return Vector3{ max.x - min.x, max.y - min.y, max.z - min.z };
}

} // namespace app

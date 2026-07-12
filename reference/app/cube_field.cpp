#include "app/cube_field.h"

#include "allocators/aligned_allocator.h"

#include <limits.h>
#include <math.h>
#include <string.h>

namespace app {

static_assert(sizeof(Cube_Visual_Style) == 8u,
              "Cube_Visual_Style must remain one adjacent fill and edge pair");
static_assert(sizeof(Cube_Value) == 1u, "Cube_Value must remain one byte per cube");

static const char* CUBE_VALUE_LABELS[CUBE_VALUE_COUNT] = {"", "A", "B", "C", "D"};

static uint32_t cube_value_hash(uint32_t x, uint32_t y, uint32_t z) {
    uint32_t hash = x * 73856093u ^ y * 19349663u ^ z * 83492791u;
    hash ^= hash >> 16u;
    hash *= 0x7feb352du;
    hash ^= hash >> 15u;
    hash *= 0x846ca68bu;
    hash ^= hash >> 16u;
    return hash;
}

int cube_field_initialize(Cube_Field& field, const Cube_Field_Config& config) {
    memset(&field, 0, sizeof(field));
    if (config.dimensions.x == 0u || config.dimensions.y == 0u || config.dimensions.z == 0u ||
        config.cube_size <= 0.0f || config.spacing <= 0.0f) {
        return 0;
    }

    uint64_t stride_z = (uint64_t)config.dimensions.x * config.dimensions.y;
    uint64_t cube_count = stride_z * config.dimensions.z;
    if (stride_z > UINT32_MAX || cube_count == 0u || cube_count >= UINT32_MAX) {
        return 0;
    }

    field.dimensions = config.dimensions;
    field.stride_y = config.dimensions.x;
    field.stride_z = (uint32_t)stride_z;
    field.cube_count = (uint32_t)cube_count;
    field.cube_size = config.cube_size;
    field.spacing = config.spacing;
    field.first_center.x = -0.5f * (float)(field.dimensions.x - 1u) * field.spacing;
    field.first_center.y = -0.5f * (float)(field.dimensions.y - 1u) * field.spacing;
    field.first_center.z = -0.5f * (float)(field.dimensions.z - 1u) * field.spacing;

    float half_cube_size = 0.5f * field.cube_size;
    field.bounds_min.x = field.first_center.x - half_cube_size;
    field.bounds_min.y = field.first_center.y - half_cube_size;
    field.bounds_min.z = field.first_center.z - half_cube_size;
    field.bounds_max.x =
        field.first_center.x + (float)(field.dimensions.x - 1u) * field.spacing + half_cube_size;
    field.bounds_max.y =
        field.first_center.y + (float)(field.dimensions.y - 1u) * field.spacing + half_cube_size;
    field.bounds_max.z =
        field.first_center.z + (float)(field.dimensions.z - 1u) * field.spacing + half_cube_size;
    return 1;
}

int cube_field_contains(const Cube_Field& field, Grid_Coordinate coordinate) {
    return coordinate.x < field.dimensions.x && coordinate.y < field.dimensions.y &&
           coordinate.z < field.dimensions.z;
}

Cube_Handle cube_field_handle(const Cube_Field& field, Grid_Coordinate coordinate) {
    if (!cube_field_contains(field, coordinate)) {
        return CUBE_HANDLE_NONE;
    }

    return coordinate.x + coordinate.y * field.stride_y + coordinate.z * field.stride_z +
           CUBE_HANDLE_FIRST;
}

int cube_field_contains_handle(const Cube_Field& field, Cube_Handle cube) {
    return cube != CUBE_HANDLE_NONE && cube <= field.cube_count;
}

Grid_Coordinate cube_field_coordinate(const Cube_Field& field, Cube_Handle cube) {
    Grid_Coordinate coordinate = {};
    if (!cube_field_contains_handle(field, cube) || field.stride_y == 0u || field.stride_z == 0u) {
        return coordinate;
    }

    uint32_t index = cube - CUBE_HANDLE_FIRST;
    coordinate.z = index / field.stride_z;
    uint32_t plane_index = index % field.stride_z;
    coordinate.y = plane_index / field.stride_y;
    coordinate.x = plane_index % field.stride_y;
    return coordinate;
}

Vector3 cube_field_coordinate_center(const Cube_Field& field, Grid_Coordinate coordinate) {
    Vector3 center = {};
    if (!cube_field_contains(field, coordinate)) {
        return center;
    }

    center.x = field.first_center.x + (float)coordinate.x * field.spacing;
    center.y = field.first_center.y + (float)coordinate.y * field.spacing;
    center.z = field.first_center.z + (float)coordinate.z * field.spacing;
    return center;
}

Vector3 cube_field_cube_center(const Cube_Field& field, Cube_Handle cube) {
    if (!cube_field_contains_handle(field, cube)) {
        return Vector3{};
    }

    return cube_field_coordinate_center(field, cube_field_coordinate(field, cube));
}

Vector3 cube_field_center(const Cube_Field& field) {
    Vector3 center = {};
    if (field.cube_count == 0u) {
        return center;
    }

    center.x = 0.5f * (field.bounds_min.x + field.bounds_max.x);
    center.y = 0.5f * (field.bounds_min.y + field.bounds_max.y);
    center.z = 0.5f * (field.bounds_min.z + field.bounds_max.z);
    return center;
}

Vector3 cube_field_half_extents(const Cube_Field& field) {
    Vector3 half_extents = {};
    if (field.cube_count == 0u) {
        return half_extents;
    }

    half_extents.x = 0.5f * (field.bounds_max.x - field.bounds_min.x);
    half_extents.y = 0.5f * (field.bounds_max.y - field.bounds_min.y);
    half_extents.z = 0.5f * (field.bounds_max.z - field.bounds_min.z);
    return half_extents;
}

void cube_field_cube_bounds(const Cube_Field& field, Cube_Handle cube, Vector3& bounds_min,
                            Vector3& bounds_max) {
    bounds_min = Vector3{};
    bounds_max = Vector3{};
    if (!cube_field_contains_handle(field, cube)) {
        return;
    }

    Vector3 center = cube_field_cube_center(field, cube);
    float half_cube_size = 0.5f * field.cube_size;
    bounds_min =
        Vector3{center.x - half_cube_size, center.y - half_cube_size, center.z - half_cube_size};
    bounds_max =
        Vector3{center.x + half_cube_size, center.y + half_cube_size, center.z + half_cube_size};
}

Cube_Handle cube_field_world_handle(const Cube_Field& field, Vector3 world_position) {
    if (field.cube_count == 0u || field.spacing <= 0.0f) {
        return CUBE_HANDLE_NONE;
    }

    if (world_position.x < field.bounds_min.x || world_position.x > field.bounds_max.x ||
        world_position.y < field.bounds_min.y || world_position.y > field.bounds_max.y ||
        world_position.z < field.bounds_min.z || world_position.z > field.bounds_max.z) {
        return CUBE_HANDLE_NONE;
    }

    int64_t x = (int64_t)floorf((world_position.x - field.first_center.x) / field.spacing + 0.5f);
    int64_t y = (int64_t)floorf((world_position.y - field.first_center.y) / field.spacing + 0.5f);
    int64_t z = (int64_t)floorf((world_position.z - field.first_center.z) / field.spacing + 0.5f);
    if (x < 0 || y < 0 || z < 0 || (uint64_t)x >= field.dimensions.x ||
        (uint64_t)y >= field.dimensions.y || (uint64_t)z >= field.dimensions.z) {
        return CUBE_HANDLE_NONE;
    }

    return cube_field_handle(field, Grid_Coordinate{(uint32_t)x, (uint32_t)y, (uint32_t)z});
}

int cube_data_initialize(Cube_Data& data, const Cube_Field& field, alloc::Allocator& allocator) {
    memset(&data, 0, sizeof(data));
    if (field.cube_count == 0u || !alloc::allocator_is_valid(allocator)) {
        return 0;
    }

    size_t allocation_size = (size_t)field.cube_count + CUBE_HANDLE_FIRST;
    data.values = (uint8_t*)alloc::allocator_allocate(allocator, allocation_size,
                                                      alloc::ALLOCATOR_CACHE_LINE_SIZE);
    if (data.values == 0) {
        return 0;
    }

    data.allocation_size = allocation_size;
    data.allocation_alignment = alloc::ALLOCATOR_CACHE_LINE_SIZE;
    data.cube_count = field.cube_count;
    data.values[CUBE_HANDLE_NONE] = CUBE_VALUE_NONE;

    Cube_Handle cube = CUBE_HANDLE_FIRST;
    for (uint32_t z = 0u; z < field.dimensions.z; ++z) {
        for (uint32_t y = 0u; y < field.dimensions.y; ++y) {
            for (uint32_t x = 0u; x < field.dimensions.x; ++x) {
                data.values[cube] = (uint8_t)(CUBE_VALUE_A + (cube_value_hash(x, y, z) & 3u));
                ++cube;
            }
        }
    }

    return 1;
}

void cube_data_shutdown(Cube_Data& data, alloc::Allocator& allocator) {
    alloc::allocator_release(allocator, data.values, data.allocation_size,
                             data.allocation_alignment);
    memset(&data, 0, sizeof(data));
}

uint8_t cube_data_value(const Cube_Data& data, Cube_Handle cube) {
    if (data.values == 0 || cube == CUBE_HANDLE_NONE || cube > data.cube_count) {
        return data.zero_stub;
    }

    return data.values[cube];
}

void cube_palette_set_initialize(Cube_Palette_Set& set) {
    memset(&set, 0, sizeof(set));

    set.palettes[CUBE_PALETTE_HANDLE_DEFAULT].name = "default";
    set.palettes[CUBE_PALETTE_HANDLE_DEFAULT].styles[CUBE_VALUE_A] =
        Cube_Visual_Style{BLUE, Color{18, 52, 150, 255}};
    set.palettes[CUBE_PALETTE_HANDLE_DEFAULT].styles[CUBE_VALUE_B] =
        Cube_Visual_Style{RED, Color{140, 18, 32, 255}};
    set.palettes[CUBE_PALETTE_HANDLE_DEFAULT].styles[CUBE_VALUE_C] =
        Cube_Visual_Style{GREEN, Color{18, 120, 44, 255}};
    set.palettes[CUBE_PALETTE_HANDLE_DEFAULT].styles[CUBE_VALUE_D] =
        Cube_Visual_Style{Color{255, 255, 255, 153}, Color{170, 180, 198, 255}};

    set.palettes[CUBE_PALETTE_HANDLE_PASTEL].name = "pastel";
    set.palettes[CUBE_PALETTE_HANDLE_PASTEL].styles[CUBE_VALUE_A] =
        Cube_Visual_Style{SKYBLUE, DARKBLUE};
    set.palettes[CUBE_PALETTE_HANDLE_PASTEL].styles[CUBE_VALUE_B] =
        Cube_Visual_Style{ORANGE, MAROON};
    set.palettes[CUBE_PALETTE_HANDLE_PASTEL].styles[CUBE_VALUE_C] =
        Cube_Visual_Style{LIME, DARKGREEN};
    set.palettes[CUBE_PALETTE_HANDLE_PASTEL].styles[CUBE_VALUE_D] =
        Cube_Visual_Style{Color{255, 255, 255, 153}, DARKGRAY};

    set.palettes[CUBE_PALETTE_HANDLE_WARM].name = "warm";
    set.palettes[CUBE_PALETTE_HANDLE_WARM].styles[CUBE_VALUE_A] = Cube_Visual_Style{PURPLE, VIOLET};
    set.palettes[CUBE_PALETTE_HANDLE_WARM].styles[CUBE_VALUE_B] = Cube_Visual_Style{GOLD, ORANGE};
    set.palettes[CUBE_PALETTE_HANDLE_WARM].styles[CUBE_VALUE_C] =
        Cube_Visual_Style{Color{0, 210, 170, 255}, Color{0, 92, 78, 255}};
    set.palettes[CUBE_PALETTE_HANDLE_WARM].styles[CUBE_VALUE_D] =
        Cube_Visual_Style{Color{255, 255, 255, 153}, Color{105, 92, 135, 255}};
    set.active_palette = CUBE_PALETTE_HANDLE_DEFAULT;
}

void cube_palette_set_select(Cube_Palette_Set& set, Cube_Palette_Handle palette) {
    if (palette >= CUBE_PALETTE_CAPACITY) {
        palette = CUBE_PALETTE_HANDLE_NONE;
    }

    set.active_palette = palette;
}

const Cube_Palette& cube_palette_set_get(const Cube_Palette_Set& set, Cube_Palette_Handle palette) {
    if (palette >= CUBE_PALETTE_CAPACITY) {
        palette = CUBE_PALETTE_HANDLE_NONE;
    }

    return set.palettes[palette];
}

const Cube_Palette& cube_palette_set_active(const Cube_Palette_Set& set) {
    return cube_palette_set_get(set, set.active_palette);
}

const Cube_Visual_Style& cube_palette_style(const Cube_Palette& palette, uint8_t value) {
    if (value >= CUBE_VALUE_COUNT) {
        value = CUBE_VALUE_NONE;
    }

    return palette.styles[value];
}

const char* cube_value_label(uint8_t value) {
    if (value >= CUBE_VALUE_COUNT) {
        value = CUBE_VALUE_NONE;
    }

    return CUBE_VALUE_LABELS[value];
}

} // namespace app

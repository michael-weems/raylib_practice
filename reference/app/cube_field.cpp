#include "app/cube_field.h"

#include <math.h>

namespace app {

static uint32_t hash_u32(uint32_t x)
{
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; x ^= x >> 16;
    return x;
}

static uint32_t clamp_u32(int v, uint32_t hi)
{
    if (v < 0) return 0;
    return (uint32_t)v > hi ? hi : (uint32_t)v;
}

void cube_field_init(Cube_Field& f, uint32_t x, uint32_t y, uint32_t z, float cube_size, float spacing)
{
    f.count_x = x; f.count_y = y; f.count_z = z; f.stride_y = x; f.stride_z = x*y; f.total_count = x*y*z;
    f.cube_size = cube_size; f.spacing = spacing;
    f.first_center.x = -((float)x - 1.0f)*spacing*0.5f;
    f.first_center.y = -((float)y - 1.0f)*spacing*0.5f;
    f.first_center.z = -((float)z - 1.0f)*spacing*0.5f;
    Vector3 last = cube_center(f, x ? x - 1u : 0u, y ? y - 1u : 0u, z ? z - 1u : 0u);
    float h = cube_size*0.5f;
    f.bounds_min = Vector3{f.first_center.x - h, f.first_center.y - h, f.first_center.z - h};
    f.bounds_max = Vector3{last.x + h, last.y + h, last.z + h};
}

Cube_Handle cube_handle_from_coords(const Cube_Field& f, uint32_t x, uint32_t y, uint32_t z)
{
    if (x >= f.count_x || y >= f.count_y || z >= f.count_z) return CUBE_HANDLE_STUB;
    return (Cube_Handle)(z*f.stride_z + y*f.stride_y + x + CUBE_HANDLE_FIRST);
}

Cube_Coords cube_coords_from_handle(const Cube_Field& f, Cube_Handle h)
{
    Cube_Coords c = {};
    if (h == CUBE_HANDLE_STUB || h > f.total_count) return c;
    uint32_t i = h - CUBE_HANDLE_FIRST;
    c.x = i%f.stride_y;
    c.y = (i/f.stride_y)%f.count_y;
    c.z = i/f.stride_z;
    return c;
}

Cube_Handle cube_handle_from_world(const Cube_Field& f, Vector3 p)
{
    if (!f.count_x || !f.count_y || !f.count_z || f.spacing == 0.0f) return CUBE_HANDLE_STUB;
    int x = (int)floorf((p.x - f.first_center.x)/f.spacing + 0.5f);
    int y = (int)floorf((p.y - f.first_center.y)/f.spacing + 0.5f);
    int z = (int)floorf((p.z - f.first_center.z)/f.spacing + 0.5f);
    return cube_handle_from_coords(f, clamp_u32(x, f.count_x - 1u), clamp_u32(y, f.count_y - 1u), clamp_u32(z, f.count_z - 1u));
}

Vector3 cube_center(const Cube_Field& f, uint32_t x, uint32_t y, uint32_t z)
{
    return Vector3{f.first_center.x + (float)x*f.spacing, f.first_center.y + (float)y*f.spacing, f.first_center.z + (float)z*f.spacing};
}

void cube_bounds(const Cube_Field& f, Cube_Handle h, Vector3& mn, Vector3& mx)
{
    Cube_Coords c = cube_coords_from_handle(f, h);
    Vector3 center = cube_center(f, c.x, c.y, c.z);
    float r = f.cube_size*0.5f;
    mn = Vector3{center.x-r, center.y-r, center.z-r};
    mx = Vector3{center.x+r, center.y+r, center.z+r};
}

int cube_data_generate(Cube_Data& d, const Cube_Field& f, alloc::Allocator& allocator)
{
    d.count = f.total_count + CUBE_HANDLE_FIRST;
    d.values = (uint8_t*)alloc::allocator_alloc(allocator, sizeof(uint8_t)*d.count, 64);
    if (!d.values) return 1;
    d.values[CUBE_HANDLE_STUB] = CUBE_VALUE_NONE;

    for (uint32_t z = 0; z < f.count_z; ++z) for (uint32_t y = 0; y < f.count_y; ++y) for (uint32_t x = 0; x < f.count_x; ++x) {
        uint32_t h = hash_u32(x*73856093u ^ y*19349663u ^ z*83492791u ^ (x*y + z*17u));
        d.values[cube_handle_from_coords(f, x, y, z)] = (uint8_t)(CUBE_VALUE_A + h%4u);
    }

    return 0;
}

void cube_data_release(Cube_Data& d, alloc::Allocator& allocator)
{
    alloc::allocator_free(allocator, d.values);
    d = Cube_Data{};
}

Cube_Value cube_value_at(const Cube_Data& d, Cube_Handle h)
{
    return h < d.count ? (Cube_Value)d.values[h] : CUBE_VALUE_NONE;
}

const char* cube_value_name(Cube_Value v)
{
    static const char* names[CUBE_VALUE_COUNT] = {"-", "A", "B", "C", "D"};
    return (uint32_t)v < CUBE_VALUE_COUNT ? names[v] : "-";
}

void cube_palettes_init(Cube_Palette_Table& t)
{
    t.count = 3;
    t.palettes[0].name = "default";
    t.palettes[0].fill[0] = BLANK; t.palettes[0].fill[1] = BLUE; t.palettes[0].fill[2] = RED; t.palettes[0].fill[3] = GREEN; t.palettes[0].fill[4] = Color{255,255,255,153};
    t.palettes[0].edge[0] = BLANK; t.palettes[0].edge[1] = SKYBLUE; t.palettes[0].edge[2] = MAROON; t.palettes[0].edge[3] = LIME; t.palettes[0].edge[4] = LIGHTGRAY;
    t.palettes[1].name = "pastel";
    t.palettes[1].fill[0] = BLANK; t.palettes[1].fill[1] = Color{70,130,255,255}; t.palettes[1].fill[2] = Color{255,95,95,255}; t.palettes[1].fill[3] = Color{80,220,120,255}; t.palettes[1].fill[4] = Color{255,255,255,153};
    t.palettes[1].edge[0] = BLANK; t.palettes[1].edge[1] = Color{20,70,210,255}; t.palettes[1].edge[2] = Color{190,30,30,255}; t.palettes[1].edge[3] = Color{20,150,70,255}; t.palettes[1].edge[4] = Color{130,130,130,255};
    t.palettes[2].name = "warm";
    t.palettes[2].fill[0] = BLANK; t.palettes[2].fill[1] = Color{30,160,220,255}; t.palettes[2].fill[2] = Color{240,110,40,255}; t.palettes[2].fill[3] = Color{180,220,60,255}; t.palettes[2].fill[4] = Color{255,255,255,153};
    t.palettes[2].edge[0] = BLANK; t.palettes[2].edge[1] = Color{10,80,130,255}; t.palettes[2].edge[2] = Color{150,50,15,255}; t.palettes[2].edge[3] = Color{90,140,20,255}; t.palettes[2].edge[4] = Color{150,150,150,255};
}

const Cube_Palette& cube_palette(const Cube_Palette_Table& table, uint32_t index)
{
    return table.palettes[table.count ? index%table.count : 0];
}

}

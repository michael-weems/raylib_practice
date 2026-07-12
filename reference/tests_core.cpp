#include "allocators/aligned_allocator.h"
#include "allocators/allocator.h"
#include "allocators/arena_allocator.h"
#include "app/cube_field.h"
#include "app/cube_view.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failure_count = 0;

static void expect_true(int condition, const char* message) {
    if (condition) {
        return;
    }

    ++failure_count;
    fprintf(stderr, "FAIL: %s\n", message);
}

static void expect_u32(uint32_t actual, uint32_t expected, const char* message) {
    if (actual == expected) {
        return;
    }

    ++failure_count;
    fprintf(stderr, "FAIL: %s (actual=%u expected=%u)\n", message, (unsigned int)actual,
            (unsigned int)expected);
}

static void expect_size(size_t actual, size_t expected, const char* message) {
    if (actual == expected) {
        return;
    }

    ++failure_count;
    fprintf(stderr, "FAIL: %s (actual=%zu expected=%zu)\n", message, actual, expected);
}

static void expect_float(float actual, float expected, const char* message) {
    float difference = actual - expected;
    if (difference < 0.0f) {
        difference = -difference;
    }
    if (difference <= 0.0001f) {
        return;
    }

    ++failure_count;
    fprintf(stderr, "FAIL: %s (actual=%f expected=%f)\n", message, actual, expected);
}

struct Tracking_Allocator_State {
    uint8_t memory[512];
    size_t allocation_size;
    size_t allocation_alignment;
    size_t release_size;
    size_t release_alignment;
    void* released_memory;
    uint32_t allocation_count;
    uint32_t release_count;
};

static void* tracking_allocate(void* context, size_t size, size_t alignment) {
    Tracking_Allocator_State& state = *(Tracking_Allocator_State*)context;
    state.allocation_size = size;
    state.allocation_alignment = alignment;
    ++state.allocation_count;

    if (alignment == 0u || (alignment & (alignment - 1u)) != 0u) {
        return 0;
    }

    uintptr_t first_address = (uintptr_t)state.memory;
    uintptr_t alignment_mask = alignment - 1u;
    uintptr_t aligned_address = (first_address + alignment_mask) & ~alignment_mask;
    size_t padding = (size_t)(aligned_address - first_address);
    if (padding > sizeof(state.memory) || size > sizeof(state.memory) - padding) {
        return 0;
    }

    return (void*)aligned_address;
}

static void tracking_release(void* context, void* memory, size_t size, size_t alignment) {
    Tracking_Allocator_State& state = *(Tracking_Allocator_State*)context;
    state.released_memory = memory;
    state.release_size = size;
    state.release_alignment = alignment;
    ++state.release_count;
}

static alloc::Allocator tracking_allocator_create(Tracking_Allocator_State& state) {
    alloc::Allocator allocator = {};
    allocator.context = &state;
    allocator.allocate = tracking_allocate;
    allocator.release = tracking_release;
    return allocator;
}

static void test_zero_state(void) {
    alloc::Allocator allocator = {};
    alloc::Arena_Allocator arena = {};
    app::Cube_Field field = {};
    app::Cube_Data data = {};
    app::Cube_Palette_Set palettes = {};
    app::Birds_Eye_Lattice lattice = {};

    expect_true(!alloc::allocator_is_valid(allocator), "zero allocator is invalid and inert");
    expect_true(alloc::allocator_allocate(allocator, 64u, 16u) == 0, "zero allocator returns null");
    alloc::allocator_release(allocator, (void*)1, 64u, 16u);
    alloc::arena_allocator_reset(arena);
    expect_size(alloc::arena_allocator_remaining(arena), 0u, "zero arena has no remaining memory");
    expect_u32(app::cube_field_handle(field, app::Grid_Coordinate{1u, 2u, 3u}),
               app::CUBE_HANDLE_NONE, "zero field returns zero handle");
    expect_u32(app::cube_data_value(data, 99u), app::CUBE_VALUE_NONE,
               "zero cube data returns zero value stub");
    expect_u32(app::cube_palette_set_active(palettes).styles[app::CUBE_VALUE_A].fill_color.a, 0u,
               "zero palette set returns transparent style stub");
    expect_u32(app::cube_view_birds_eye_shell_sample_count(lattice), 0u,
               "zero lattice has no shell samples");
}

static void test_allocator_contract(void) {
    Tracking_Allocator_State tracking_state = {};
    alloc::Allocator tracking_allocator = tracking_allocator_create(tracking_state);
    expect_true(alloc::allocator_is_valid(tracking_allocator),
                "complete allocator callback pair is valid");
    expect_true(alloc::allocator_allocate(tracking_allocator, 0u, 64u) == 0,
                "zero-size allocation is inert");
    expect_u32(tracking_state.allocation_count, 0u,
               "zero-size allocation does not dispatch callback");

    void* memory = alloc::allocator_allocate(tracking_allocator, 96u, 64u);
    expect_true(memory != 0, "allocator dispatches allocation");
    expect_true(((uintptr_t)memory & 63u) == 0u, "allocator receives cache-line alignment");
    alloc::allocator_release(tracking_allocator, memory, 96u, 64u);
    expect_u32(tracking_state.allocation_count, 1u, "allocator records one allocation");
    expect_u32(tracking_state.release_count, 1u, "allocator records one release");
    expect_size(tracking_state.release_size, 96u, "release receives allocation size");
    expect_size(tracking_state.release_alignment, 64u, "release receives allocation alignment");
    expect_true(tracking_state.released_memory == memory, "release receives allocation address");

    alloc::Allocator aligned_allocator = alloc::aligned_allocator_create();
    void* aligned_memory = alloc::allocator_allocate(aligned_allocator, 257u, 64u);
    expect_true(aligned_memory != 0, "aligned persistent allocator allocates");
    expect_true(((uintptr_t)aligned_memory & 63u) == 0u,
                "persistent allocation is cache-line aligned");
    expect_true(alloc::allocator_allocate(aligned_allocator, 16u, 3u) == 0,
                "persistent allocator rejects non-power-of-two alignment");
    alloc::allocator_release(aligned_allocator, aligned_memory, 257u, 64u);
}

static void test_arena_allocator(void) {
    uint8_t backing_memory[385] = {};
    alloc::Arena_Allocator arena = {};
    expect_true(!alloc::arena_allocator_initialize(arena, 0, 384u),
                "arena rejects null backing memory");
    expect_true(!alloc::arena_allocator_initialize(arena, backing_memory, 0u),
                "arena rejects empty backing memory");
    expect_true(alloc::arena_allocator_initialize(arena, backing_memory + 1u, 384u),
                "arena binds caller-owned storage");

    alloc::Allocator interface_allocator = alloc::arena_allocator_create_interface(arena);
    void* first = alloc::allocator_allocate(interface_allocator, 31u, 16u);
    void* second = alloc::allocator_allocate(interface_allocator, 65u, 64u);
    expect_true(first != 0 && ((uintptr_t)first & 15u) == 0u,
                "arena returns 16-byte aligned memory");
    expect_true(second != 0 && ((uintptr_t)second & 63u) == 0u,
                "arena returns 64-byte aligned memory");
    expect_true(alloc::arena_allocator_allocate(arena, 8u, 3u) == 0,
                "arena rejects non-power-of-two alignment");
    expect_true(alloc::arena_allocator_allocate(arena, 1024u, 16u) == 0,
                "arena exhaustion has no fallback");
    size_t high_water_mark = arena.high_water_mark;
    expect_true(high_water_mark >= 96u, "arena records high-water mark");

    alloc::allocator_release(interface_allocator, first, 31u, 16u);
    expect_true(arena.used == high_water_mark, "individual arena release is intentionally inert");
    alloc::arena_allocator_reset(arena);
    expect_size(arena.used, 0u, "arena reset reclaims all allocations");
    expect_size(arena.high_water_mark, high_water_mark, "arena reset preserves high-water mark");
    expect_size(alloc::arena_allocator_remaining(arena), arena.capacity,
                "reset restores full arena capacity");

    alloc::Arena_Allocator overflow_arena = {};
    overflow_arena.memory = (uint8_t*)(UINTPTR_MAX - 7u);
    overflow_arena.capacity = 64u;
    expect_true(alloc::arena_allocator_allocate(overflow_arena, 8u, 16u) == 0,
                "arena rejects address alignment overflow");
}

static app::Cube_Field make_field(uint32_t x, uint32_t y, uint32_t z, float cube_size,
                                  float spacing) {
    app::Cube_Field field = {};
    app::Cube_Field_Config config = {};
    config.dimensions = app::Grid_Dimensions{x, y, z};
    config.cube_size = cube_size;
    config.spacing = spacing;
    expect_true(app::cube_field_initialize(field, config), "test field initializes");
    return field;
}

static void test_cube_field_and_handles(void) {
    app::Cube_Field field = make_field(4u, 3u, 2u, 1.0f, 5.0f);
    expect_u32(field.stride_y, 4u, "x is contiguous within y stride");
    expect_u32(field.stride_z, 12u, "z stride equals x times y");
    expect_u32(field.cube_count, 24u, "cube count excludes zero stub");
    expect_u32(app::cube_field_handle(field, app::Grid_Coordinate{0u, 0u, 0u}), 1u,
               "first cube has handle one");
    expect_u32(app::cube_field_handle(field, app::Grid_Coordinate{1u, 0u, 0u}), 2u,
               "x advances contiguous handle");
    expect_u32(app::cube_field_handle(field, app::Grid_Coordinate{0u, 0u, 1u}), 13u,
               "z advances one complete plane");
    expect_u32(app::cube_field_handle(field, app::Grid_Coordinate{3u, 2u, 1u}), 24u,
               "last coordinate maps to cube count");
    expect_u32(app::cube_field_handle(field, app::Grid_Coordinate{4u, 0u, 0u}), 0u,
               "out-of-range coordinate maps to zero");
    expect_true(!app::cube_field_contains_handle(field, 0u), "zero handle is not a real cube");
    expect_true(!app::cube_field_contains_handle(field, 25u), "past-end handle is rejected");

    for (uint32_t z = 0u; z < field.dimensions.z; ++z) {
        for (uint32_t y = 0u; y < field.dimensions.y; ++y) {
            for (uint32_t x = 0u; x < field.dimensions.x; ++x) {
                app::Grid_Coordinate source_coordinate = {x, y, z};
                app::Cube_Handle cube = app::cube_field_handle(field, source_coordinate);
                app::Grid_Coordinate recovered_coordinate = app::cube_field_coordinate(field, cube);
                expect_true(recovered_coordinate.x == x && recovered_coordinate.y == y &&
                                recovered_coordinate.z == z,
                            "every coordinate and handle round-trip");
            }
        }
    }

    Vector3 first_center =
        app::cube_field_coordinate_center(field, app::Grid_Coordinate{0u, 0u, 0u});
    Vector3 last_center =
        app::cube_field_coordinate_center(field, app::Grid_Coordinate{3u, 2u, 1u});
    expect_float(first_center.x, -7.5f, "even x minimum is centered");
    expect_float(last_center.x, 7.5f, "even x maximum is centered");
    expect_float(first_center.y, -5.0f, "odd y minimum is centered");
    expect_float(last_center.y, 5.0f, "odd y maximum is centered");
    expect_float(first_center.z, -2.5f, "even z minimum is centered");
    expect_float(last_center.z, 2.5f, "even z maximum is centered");
    expect_float(app::cube_field_center(field).x, 0.0f, "field center is world origin");
    expect_float(app::cube_field_half_extents(field).x, 8.0f,
                 "half extent includes half cube size");
    expect_u32(app::cube_field_world_handle(field, first_center), 1u,
               "world-space center resolves to same handle");
    expect_u32(app::cube_field_world_handle(field, Vector3{100.0f, 0.0f, 0.0f}), 0u,
               "world position outside field resolves to zero");

    app::Cube_Field odd_field = make_field(3u, 5u, 7u, 2.0f, 4.0f);
    Vector3 odd_middle_center =
        app::cube_field_coordinate_center(odd_field, app::Grid_Coordinate{1u, 2u, 3u});
    expect_float(odd_middle_center.x, 0.0f, "odd x middle cube is centered");
    expect_float(odd_middle_center.y, 0.0f, "odd y middle cube is centered");
    expect_float(odd_middle_center.z, 0.0f, "odd z middle cube is centered");

    app::Cube_Field invalid_field = field;
    app::Cube_Field_Config invalid_config = {};
    invalid_config.dimensions = app::Grid_Dimensions{UINT32_MAX, UINT32_MAX, 2u};
    invalid_config.cube_size = 1.0f;
    invalid_config.spacing = 1.0f;
    expect_true(!app::cube_field_initialize(invalid_field, invalid_config),
                "field rejects handle-count overflow");
    expect_u32(invalid_field.cube_count, 0u, "failed initialization restores zero field");
}

static void test_cube_data_and_palettes(void) {
    app::Cube_Field field = make_field(4u, 3u, 2u, 1.0f, 5.0f);
    alloc::Allocator allocator = alloc::aligned_allocator_create();
    app::Cube_Data first_data = {};
    app::Cube_Data second_data = {};
    expect_true(app::cube_data_initialize(first_data, field, allocator),
                "first immutable value stream initializes");
    expect_true(app::cube_data_initialize(second_data, field, allocator),
                "second immutable value stream initializes");
    expect_true(((uintptr_t)first_data.values & 63u) == 0u,
                "cube values start on cache-line boundary");
    expect_size(first_data.allocation_size, field.cube_count + 1u,
                "cube values allocate one byte per handle including stub");
    expect_u32(app::cube_data_value(first_data, 0u), app::CUBE_VALUE_NONE,
               "zero handle resolves to zero value stub");
    expect_u32(app::cube_data_value(first_data, field.cube_count + 1u), app::CUBE_VALUE_NONE,
               "invalid handle resolves to zero value stub");
    for (app::Cube_Handle cube = 1u; cube <= field.cube_count; ++cube) {
        uint8_t first_value = app::cube_data_value(first_data, cube);
        uint8_t second_value = app::cube_data_value(second_data, cube);
        expect_true(first_value >= app::CUBE_VALUE_A && first_value <= app::CUBE_VALUE_D,
                    "every real cube has A, B, C, or D value");
        expect_u32(first_value, second_value, "cube value generation is deterministic");
    }
    app::cube_data_shutdown(first_data, allocator);
    app::cube_data_shutdown(second_data, allocator);
    expect_true(first_data.values == 0 && first_data.cube_count == 0u,
                "cube data shutdown restores zero state");

    app::Cube_Palette_Set palettes = {};
    app::cube_palette_set_initialize(palettes);
    expect_u32(palettes.active_palette, app::CUBE_PALETTE_HANDLE_DEFAULT,
               "palette initialization selects first real palette");
    const app::Cube_Palette& zero_palette =
        app::cube_palette_set_get(palettes, app::CUBE_PALETTE_HANDLE_NONE);
    expect_u32(zero_palette.styles[app::CUBE_VALUE_A].fill_color.a, 0u,
               "palette handle zero has all-zero fill");
    expect_u32(zero_palette.styles[app::CUBE_VALUE_A].edge_color.a, 0u,
               "palette handle zero has all-zero edge");
    const app::Cube_Palette& default_palette = app::cube_palette_set_active(palettes);
    expect_u32(default_palette.styles[app::CUBE_VALUE_A].fill_color.b, BLUE.b,
               "A is blue in default palette");
    expect_u32(default_palette.styles[app::CUBE_VALUE_B].fill_color.r, RED.r,
               "B is red in default palette");
    expect_u32(default_palette.styles[app::CUBE_VALUE_C].fill_color.g, GREEN.g,
               "C is green in default palette");
    expect_u32(default_palette.styles[app::CUBE_VALUE_D].fill_color.a, 153u,
               "D is sixty-percent opaque white");
    expect_true(memcmp(&default_palette.styles[app::CUBE_VALUE_A].fill_color,
                       &default_palette.styles[app::CUBE_VALUE_A].edge_color, sizeof(Color)) != 0,
                "palette edge color differs from fill color");
    app::cube_palette_set_select(palettes, app::CUBE_PALETTE_HANDLE_WARM);
    expect_true(app::cube_palette_set_active(palettes).name != 0,
                "real palette handle resolves inline palette");
    app::cube_palette_set_select(palettes, 99u);
    expect_u32(palettes.active_palette, app::CUBE_PALETTE_HANDLE_NONE,
               "invalid palette handle collapses to zero stub");
    expect_true(strcmp(app::cube_value_label(app::CUBE_VALUE_C), "C") == 0,
                "cube value label resolves stable text");
    expect_true(strcmp(app::cube_value_label(99u), "") == 0,
                "invalid cube value label resolves empty stub");
}

static void test_focused_view_math(void) {
    app::Cube_Field field = make_field(11u, 11u, 11u, 1.0f, 5.0f);
    app::Grid_Coordinate center_coordinate = {5u, 5u, 5u};
    app::Cube_Handle center_cube = app::cube_field_handle(field, center_coordinate);
    app::Grid_Region centered_region = app::cube_view_focused_region(field, center_cube, 5u);
    expect_u32(centered_region.minimum.x, 0u, "focused region reaches negative x radius");
    expect_u32(centered_region.maximum.x, 10u, "focused region reaches positive x radius");

    uint32_t accepted_count = 0u;
    for (uint32_t z = centered_region.minimum.z; z <= centered_region.maximum.z; ++z) {
        for (uint32_t y = centered_region.minimum.y; y <= centered_region.maximum.y; ++y) {
            for (uint32_t x = centered_region.minimum.x; x <= centered_region.maximum.x; ++x) {
                if (app::cube_view_focused_accepts_coordinate(center_coordinate,
                                                              app::Grid_Coordinate{x, y, z}, 5u)) {
                    ++accepted_count;
                }
            }
        }
    }
    expect_u32(accepted_count, 515u, "radius-five Euclidean region contains 515 cubes");
    expect_true(app::cube_view_focused_accepts_coordinate(center_coordinate,
                                                          app::Grid_Coordinate{8u, 9u, 5u}, 5u),
                "3-4-5 offset lies on focused sphere");
    expect_true(!app::cube_view_focused_accepts_coordinate(center_coordinate,
                                                           app::Grid_Coordinate{9u, 9u, 5u}, 5u),
                "offset beyond focused sphere is rejected");

    app::Grid_Region corner_region = app::cube_view_focused_region(field, 1u, 5u);
    expect_u32(corner_region.minimum.x, 0u, "focused region clips at field minimum");
    expect_u32(corner_region.maximum.x, 5u, "focused region preserves radius inside field");
}

static void test_birds_eye_view_math(void) {
    expect_u32(app::cube_view_sample_count(UINT32_MAX, 32u), 134217728u,
               "sample count avoids uint32 addition overflow");
    expect_u32(app::cube_view_sample_count(2u, 32u), 2u,
               "sampling preserves both endpoints of short axis");
    expect_u32(app::cube_view_source_coordinate(0u, 16u, 500u), 0u,
               "source mapping preserves first endpoint");
    expect_u32(app::cube_view_source_coordinate(15u, 16u, 500u), 499u,
               "source mapping preserves last endpoint");

    app::Cube_Field field = make_field(500u, 1000u, 100u, 1.0f, 5.0f);
    app::Cube_View_Config view_config = {};
    view_config.focused_radius = 5u;
    view_config.face_text_radius = 3u;
    view_config.birds_eye_sample_stride = 32u;
    app::Birds_Eye_Lattice lattice = {};
    app::cube_view_build_birds_eye_lattice(lattice, field, view_config);
    expect_u32(lattice.sample_count.x, 16u, "birds-eye x sampling is sparse");
    expect_u32(lattice.sample_count.y, 32u, "birds-eye y sampling is sparse");
    expect_u32(lattice.sample_count.z, 4u, "birds-eye z sampling is sparse");
    expect_float(lattice.representative_size, 160.0f,
                 "birds-eye representatives use stride-scaled edge length");
    expect_float(lattice.bounds_max.x - lattice.bounds_min.x, 2560.0f,
                 "birds-eye representatives exactly butt along x");
    expect_u32(app::cube_view_birds_eye_shell_sample_count(lattice), 1208u,
               "direct shell enumeration excludes every interior sample");

    uint8_t visited_samples[2048] = {};
    uint32_t shell_count = app::cube_view_birds_eye_shell_sample_count(lattice);
    for (uint32_t shell_index = 0u; shell_index < shell_count; ++shell_index) {
        app::Birds_Eye_Sample sample = {};
        expect_true(app::cube_view_birds_eye_shell_sample(sample, field, lattice, shell_index),
                    "every shell ordinal resolves a sample");
        uint32_t dense_index =
            sample.lattice_coordinate.x + sample.lattice_coordinate.y * lattice.sample_count.x +
            sample.lattice_coordinate.z * lattice.sample_count.x * lattice.sample_count.y;
        expect_true(visited_samples[dense_index] == 0u,
                    "shell enumerator produces each exterior sample once");
        visited_samples[dense_index] = 1u;
        expect_true(sample.outward_faces != app::BIRDS_EYE_FACE_NONE,
                    "shell sample exposes at least one outward face");
        expect_true(app::cube_field_contains_handle(field, sample.source_cube),
                    "shell sample resolves a real source handle");
        expect_u32(sample.source_cube, app::cube_field_handle(field, sample.source_coordinate),
                   "drawing and picking share exact sample source identity");
        expect_float(sample.bounds_max.x - sample.bounds_min.x, lattice.representative_size,
                     "representative bounds preserve non-overlapping edge length");
    }

    app::Birds_Eye_Sample first_corner = {};
    app::Birds_Eye_Sample last_corner = {};
    expect_true(app::cube_view_birds_eye_sample(first_corner, field, lattice,
                                                app::Grid_Coordinate{0u, 0u, 0u}),
                "first shell corner resolves");
    expect_true(app::cube_view_birds_eye_sample(last_corner, field, lattice,
                                                app::Grid_Coordinate{15u, 31u, 3u}),
                "last shell corner resolves");
    expect_u32(first_corner.source_cube, 1u, "first lattice endpoint maps to first source cube");
    expect_u32(last_corner.source_cube, field.cube_count,
               "last lattice endpoint maps to last source cube");
    expect_u32(first_corner.outward_faces,
               app::BIRDS_EYE_FACE_NEGATIVE_X | app::BIRDS_EYE_FACE_NEGATIVE_Y |
                   app::BIRDS_EYE_FACE_NEGATIVE_Z,
               "corner reports exactly three outward faces");

    app::Birds_Eye_Sample outside_sample = {};
    expect_true(!app::cube_view_birds_eye_sample(outside_sample, field, lattice,
                                                 app::Grid_Coordinate{16u, 0u, 0u}),
                "out-of-range lattice coordinate returns zero sample");
    expect_u32(outside_sample.source_cube, 0u, "failed sample remains zero initialized");

    app::Cube_Field one_cell_field = make_field(1u, 1u, 1u, 1.0f, 5.0f);
    app::Birds_Eye_Lattice one_cell_lattice = {};
    app::cube_view_build_birds_eye_lattice(one_cell_lattice, one_cell_field, view_config);
    expect_u32(app::cube_view_birds_eye_shell_sample_count(one_cell_lattice), 1u,
               "one-cell lattice has one exterior representative");
    app::Birds_Eye_Sample one_cell_sample = {};
    expect_true(app::cube_view_birds_eye_shell_sample(one_cell_sample, one_cell_field,
                                                      one_cell_lattice, 0u),
                "one-cell shell ordinal resolves safely");
    expect_u32(one_cell_sample.outward_faces,
               app::BIRDS_EYE_FACE_NEGATIVE_X | app::BIRDS_EYE_FACE_POSITIVE_X |
                   app::BIRDS_EYE_FACE_NEGATIVE_Y | app::BIRDS_EYE_FACE_POSITIVE_Y |
                   app::BIRDS_EYE_FACE_NEGATIVE_Z | app::BIRDS_EYE_FACE_POSITIVE_Z,
               "one-cell representative exposes all six outward faces");

    app::Cube_Field two_cell_field = make_field(2u, 2u, 2u, 1.0f, 5.0f);
    app::Birds_Eye_Lattice two_cell_lattice = {};
    app::cube_view_build_birds_eye_lattice(two_cell_lattice, two_cell_field, view_config);
    expect_u32(two_cell_lattice.sample_count.x, 2u, "two-cell lattice preserves both x endpoints");
    expect_u32(app::cube_view_birds_eye_shell_sample_count(two_cell_lattice), 8u,
               "two-by-two-by-two lattice enumerates eight unique shell cells");
    uint8_t two_cell_visited[8] = {};
    for (uint32_t shell_index = 0u; shell_index < 8u; ++shell_index) {
        app::Birds_Eye_Sample sample = {};
        expect_true(app::cube_view_birds_eye_shell_sample(sample, two_cell_field, two_cell_lattice,
                                                          shell_index),
                    "every two-cell shell ordinal resolves");
        uint32_t dense_index = sample.lattice_coordinate.x + sample.lattice_coordinate.y * 2u +
                               sample.lattice_coordinate.z * 4u;
        expect_true(two_cell_visited[dense_index] == 0u,
                    "two-cell shell enumeration has no duplicates");
        two_cell_visited[dense_index] = 1u;
    }

    app::Cube_Field thin_field = make_field(1u, 2u, 3u, 1.0f, 5.0f);
    app::Cube_View_Config thin_view_config = {};
    thin_view_config.birds_eye_sample_stride = 1u;
    app::Birds_Eye_Lattice thin_lattice = {};
    app::cube_view_build_birds_eye_lattice(thin_lattice, thin_field, thin_view_config);
    expect_u32(app::cube_view_birds_eye_shell_sample_count(thin_lattice), 6u,
               "single-axis-thickness lattice enumerates every cell exactly once");
    uint8_t thin_visited[6] = {};
    for (uint32_t shell_index = 0u; shell_index < 6u; ++shell_index) {
        app::Birds_Eye_Sample sample = {};
        expect_true(
            app::cube_view_birds_eye_shell_sample(sample, thin_field, thin_lattice, shell_index),
            "every single-axis-thickness shell ordinal resolves");
        uint32_t dense_index = sample.lattice_coordinate.y + sample.lattice_coordinate.z * 2u;
        expect_true(thin_visited[dense_index] == 0u,
                    "single-axis-thickness shell enumeration has no duplicates");
        thin_visited[dense_index] = 1u;
        expect_u32(sample.source_cube, app::cube_field_handle(thin_field, sample.source_coordinate),
                   "degenerate shell preserves exact source identity");
    }
    app::Birds_Eye_Sample invalid_ordinal_sample = {};
    expect_true(!app::cube_view_birds_eye_shell_sample(invalid_ordinal_sample, thin_field,
                                                       thin_lattice, 6u),
                "past-end shell ordinal is rejected");
    expect_u32(invalid_ordinal_sample.source_cube, 0u,
               "past-end shell ordinal leaves zero source stub");
}

static void test_sparse_picking(void) {
    app::Cube_Field focused_field = make_field(7u, 7u, 7u, 1.0f, 5.0f);
    app::Grid_Coordinate selected_coordinate = {3u, 3u, 3u};
    app::Cube_Handle selected_cube = app::cube_field_handle(focused_field, selected_coordinate);
    Ray focused_ray = {};
    focused_ray.position = Vector3{0.0f, 0.0f, -100.0f};
    focused_ray.direction = Vector3{0.0f, 0.0f, 1.0f};
    app::Cube_Pick_Result focused_result =
        app::cube_view_pick_focused(focused_field, selected_cube, 2u, focused_ray);
    expect_u32(focused_result.cube,
               app::cube_field_handle(focused_field, app::Grid_Coordinate{3u, 3u, 1u}),
               "focused picking tests only Euclidean sparse candidates");

    app::Cube_Field birds_eye_field = make_field(500u, 1000u, 100u, 1.0f, 5.0f);
    app::Cube_View_Config view_config = {};
    view_config.birds_eye_sample_stride = 32u;
    app::Birds_Eye_Lattice lattice = {};
    app::cube_view_build_birds_eye_lattice(lattice, birds_eye_field, view_config);
    app::Grid_Coordinate top_sample_coordinate = {3u, lattice.sample_count.y - 1u, 2u};
    app::Birds_Eye_Sample top_sample = {};
    expect_true(app::cube_view_birds_eye_sample(top_sample, birds_eye_field, lattice,
                                                top_sample_coordinate),
                "top picking sample resolves");
    Ray birds_eye_ray = {};
    birds_eye_ray.position =
        Vector3{top_sample.center.x, lattice.bounds_max.y + 1000.0f, top_sample.center.z};
    birds_eye_ray.direction = Vector3{0.0f, -1.0f, 0.0f};
    app::Cube_Pick_Result birds_eye_result =
        app::cube_view_pick_birds_eye(birds_eye_field, lattice, birds_eye_ray);
    expect_u32(birds_eye_result.cube, top_sample.source_cube,
               "birds-eye picking returns the source identity used to draw hit representative");
}

int main(void) {
    test_zero_state();
    test_allocator_contract();
    test_arena_allocator();
    test_cube_field_and_handles();
    test_cube_data_and_palettes();
    test_focused_view_math();
    test_birds_eye_view_math();
    test_sparse_picking();

    if (failure_count != 0) {
        fprintf(stderr, "%d core test(s) failed\n", failure_count);
        return 1;
    }

    printf("All reference core tests passed\n");
    return 0;
}

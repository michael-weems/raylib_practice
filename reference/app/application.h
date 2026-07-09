#pragma once

#include "allocators/arena_allocator.h"
#include "app/cube_field.h"
#include "sdk/orbit_camera.h"
#include "sdk/runtime.h"
#include "raylib.h"

#include <stdint.h>

namespace app {

enum View_Mode {
    VIEW_LOCAL = 0,
    VIEW_BIRDS_EYE = 1
};

struct Local_View_Config {
    int render_radius;
    int text_radius;
    int grid_fine_radius;
    int grid_min_major_stride;
    float clip_near;
    float clip_far;
    sdk::Orbit_Config orbit;
};

struct Birds_Eye_Config {
    uint32_t block_source_span;
    float clip_near;
    float clip_far;
    float start_distance;
    sdk::Orbit_Config orbit;
};

struct Compass_Config {
    float local_length;
    float local_radius;
    float local_label_size;
    float bird_length_scale;
    float bird_radius_scale;
    float bird_label_pixels;
};

struct Application_Config {
    sdk::Runtime_Config runtime;
    Local_View_Config local;
    Birds_Eye_Config birds_eye;
    Compass_Config compass;
    uint32_t field_x;
    uint32_t field_y;
    uint32_t field_z;
    float cube_size;
    float cube_spacing;
    size_t frame_arena_size;
};

struct Application {
    Application_Config config;
    alloc::Allocator persistent_allocator;
    alloc::Arena frame_arena;
    void* frame_memory;
    size_t frame_memory_size;
    sdk::Runtime_State runtime;
    sdk::Orbit_Camera orbit;
    sdk::Cursor_Capture cursor;
    Cube_Field field;
    Cube_Data data;
    Cube_Palette_Table palettes;
    Cube_Handle active_cube;
    View_Mode view_mode;
    uint32_t palette_index;
    Font font;
    int owns_font;
};

Application_Config application_default_config(void);
int application_init(Application& app, const Application_Config& config, alloc::Allocator& allocator);
void application_frame(Application& app);
void application_shutdown(Application& app);

}

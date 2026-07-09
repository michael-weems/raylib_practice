#ifndef REFERENCE_APP_APPLICATION_H
#define REFERENCE_APP_APPLICATION_H

#include "allocators/arena_allocator.h"
#include "app/cube_field.h"
#include "sdk/orbit_camera.h"
#include "sdk/runtime.h"

#include <stddef.h>
#include <stdint.h>

namespace app {

enum App_Result {
    APP_SUCCESS = 0,
    APP_ERROR_RUNTIME_INIT,
    APP_ERROR_CUBE_DATA,
    APP_ERROR_FRAME_ARENA_ALLOCATION
};

enum App_View_Mode {
    APP_VIEW_LOCAL = 0,
    APP_VIEW_BIRDS_EYE
};

struct App_Config {
    sdk::Runtime_Config runtime;
    Cube_Field field;
    uint32_t data_seed;
    size_t frame_arena_bytes;
};

struct App_State {
    sdk::Runtime_State runtime;
    App_Config config;

    Cube_Data cube_data;
    Cube_Palette palette;
    uint32_t active_palette_index;

    sdk::Orbit_Camera_Config local_camera_config;
    sdk::Orbit_Camera_Config bird_camera_config;
    sdk::Orbit_Camera_State local_camera;
    sdk::Orbit_Camera_State bird_camera;

    App_View_Mode view_mode;
    Cube_Handle active_cube;

    allocators::Arena_Allocator frame_arena;
    void* frame_memory;
    size_t frame_memory_size;

    int is_initialized;
};

void app_default_config(App_Config& config);

App_Result app_init(
    allocators::Allocator& persistent_allocator,
    App_Config& config,
    App_State& state);

void app_shutdown(
    allocators::Allocator& persistent_allocator,
    App_State& state);

void app_update_and_render(App_State& state);

} // namespace app

#endif // REFERENCE_APP_APPLICATION_H

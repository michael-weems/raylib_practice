#pragma once

#include "allocators/allocator.h"
#include "app/cube_field.h"
#include "app/cube_view.h"
#include "sdk/orbit_camera.h"
#include "sdk/rendering.h"
#include "sdk/runtime.h"

#include "raylib.h"

#include <stddef.h>
#include <stdint.h>

namespace app {

enum View_Mode { VIEW_MODE_NONE = 0, VIEW_MODE_LOCAL, VIEW_MODE_BIRDS_EYE, VIEW_MODE_COUNT };

enum Application_Result {
    APPLICATION_RESULT_OK = 0,
    APPLICATION_RESULT_INVALID_CONFIGURATION,
    APPLICATION_RESULT_INVALID_ALLOCATOR,
    APPLICATION_RESULT_RUNTIME_INITIALIZATION_FAILED,
    APPLICATION_RESULT_CUBE_FIELD_INITIALIZATION_FAILED,
    APPLICATION_RESULT_CUBE_DATA_INITIALIZATION_FAILED,
    APPLICATION_RESULT_RENDER_RESOURCE_INITIALIZATION_FAILED,
    APPLICATION_RESULT_TEMPORARY_MEMORY_EXHAUSTED
};

struct Face_Text_Config {
    float font_size;
    float glyph_spacing;
    float line_spacing;
    float surface_offset;
    Color color;
};

struct Local_View_Config {
    double clip_near;
    double clip_far;
    float start_distance;
    sdk::Orbit_Config orbit;
};

struct Birds_Eye_Config {
    double clip_near;
    double clip_far;
    float start_distance;
    float start_pitch;
    sdk::Orbit_Config orbit;
};

struct Compass_Config {
    float local_arrow_length;
    float local_shaft_radius;
    float local_arrowhead_length;
    float local_arrowhead_radius;
    float local_label_size;
    float local_gap_center;
    float local_gap_half_length;
    float local_surface_offset;
    float birds_eye_arrow_length_scale;
    float birds_eye_shaft_radius_scale;
    float birds_eye_arrowhead_length_scale;
    float birds_eye_arrowhead_radius_scale;
    float birds_eye_gap_half_length_scale;
    float birds_eye_surface_offset_scale;
    float birds_eye_label_pixels;
    Color axis_color[sdk::AXIS_COUNT];
    Color label_color;
    Color label_background_color;
};

struct Boundary_Grid_Config {
    uint32_t fine_radius;
    uint32_t minimum_major_stride;
    Color line_color;
    Color border_color;
};

struct Application_Config {
    sdk::Runtime_Config runtime;
    sdk::Render_Resource_Config render_resources;
    Cube_Field_Config cube_field;
    Cube_View_Config cube_view;
    Local_View_Config local;
    Birds_Eye_Config birds_eye;
    Face_Text_Config face_text;
    Compass_Config compass;
    Boundary_Grid_Config boundary_grid;
    Color clear_color;
};

struct Application_Memory_Requirements {
    alloc::Allocator_Requirements temporary;
    uint32_t maximum_face_command_count;
};

// Frequently updated camera and selection values are adjacent. Large immutable
// cube values remain in one separately allocated byte stream owned by Cube_Data.
struct alignas(64) Application {
    sdk::Orbit_Camera orbit;
    sdk::Cursor_Capture cursor;
    Cube_Handle active_cube;
    uint8_t view_mode;
    uint8_t initialized;
    uint8_t reserved[2];
    Camera3D camera;
    Cube_Field field;
    Cube_Data data;
    Cube_Palette_Set palettes;
    sdk::Render_Resources render_resources;
    sdk::Runtime_State runtime;
    Application_Config config;
};

Application_Config application_default_config(void);
Application_Result application_memory_requirements(Application_Memory_Requirements& requirements,
                                                   const Application_Config& config);
Application_Result application_init(Application& application, const Application_Config& config,
                                    alloc::Allocator& persistent_allocator);
Application_Result application_frame(Application& application,
                                     alloc::Allocator& temporary_allocator);
void application_shutdown(Application& application, alloc::Allocator& persistent_allocator);
const char* application_result_label(Application_Result result);

} // namespace app

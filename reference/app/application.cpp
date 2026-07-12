#include "app/application.h"

#include "raymath.h"
#include "rlgl.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace app {

static const size_t FACE_COMMAND_ALIGNMENT = 64u;
static_assert(alignof(Application) == 64u,
              "Application hot state must begin on a cache-line boundary");

struct Application_Input {
    Vector2 mouse_position;
    Vector2 mouse_delta;
    float mouse_wheel;
    uint8_t window_focused;
    uint8_t toggle_view;
    uint8_t toggle_cursor;
    uint8_t select_cube;
    uint8_t navigate_left;
    uint8_t navigate_right;
    uint8_t navigate_forward;
    uint8_t navigate_backward;
    uint8_t palette_1;
    uint8_t palette_2;
    uint8_t palette_3;
};

struct Birds_Eye_Label {
    const char* text;
    Vector3 world_position;
    uint8_t visible;
};

static int checked_multiply_u64(uint64_t left, uint64_t right, uint64_t& result) {
    if (left != 0u && right > UINT64_MAX / left) {
        return 0;
    }

    result = left * right;
    return 1;
}

static uint64_t minimum_u64(uint64_t left, uint64_t right) {
    if (left < right) {
        return left;
    }

    return right;
}

static float maximum_of_three(float first_value, float second_value, float third_value) {
    float maximum = first_value;
    if (second_value > maximum) {
        maximum = second_value;
    }
    if (third_value > maximum) {
        maximum = third_value;
    }

    return maximum;
}

static int clamp_int(int value, int minimum, int maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }

    return value;
}

static float face_view_depth(const Camera3D& camera, Vector3 cube_center, float edge_length,
                             uint32_t direction) {
    const sdk::Axis_Direction_Info& direction_info = sdk::axis_direction_info(direction);
    Vector3 face_center =
        Vector3Add(cube_center, Vector3Scale(direction_info.normal, 0.5f * edge_length));
    Vector3 camera_forward = Vector3Subtract(camera.target, camera.position);
    Vector3 camera_to_face = Vector3Subtract(face_center, camera.position);
    return Vector3DotProduct(camera_to_face, camera_forward);
}

static void swap_face_commands(sdk::Cube_Face_Draw& left, sdk::Cube_Face_Draw& right) {
    sdk::Cube_Face_Draw temporary = left;
    left = right;
    right = temporary;
}

static void sift_minimum_face_heap(sdk::Cube_Face_Draw* commands, uint32_t root, uint32_t count) {
    for (;;) {
        uint32_t child = root * 2u + 1u;
        uint32_t smallest = root;
        if (child < count && commands[child].sort_depth < commands[smallest].sort_depth) {
            smallest = child;
        }
        if (child + 1u < count && commands[child + 1u].sort_depth < commands[smallest].sort_depth) {
            smallest = child + 1u;
        }
        if (smallest == root) {
            return;
        }

        swap_face_commands(commands[root], commands[smallest]);
        root = smallest;
    }
}

// The min-heap moves near faces to the shrinking tail, leaving one contiguous
// far-to-near stream for software alpha blending.
static void sort_faces_far_to_near(sdk::Cube_Face_Draw* commands, uint32_t count) {
    if (commands == 0 || count < 2u) {
        return;
    }

    uint32_t root = count / 2u;
    while (root > 0u) {
        --root;
        sift_minimum_face_heap(commands, root, count);
    }

    uint32_t remaining = count;
    while (remaining > 1u) {
        --remaining;
        swap_face_commands(commands[0], commands[remaining]);
        sift_minimum_face_heap(commands, 0u, remaining);
    }
}

static uint32_t partition_transparent_faces(sdk::Cube_Face_Draw* commands, uint32_t count) {
    uint32_t first_transparent = 0u;
    while (first_transparent < count) {
        Color fill_color = commands[first_transparent].fill_color;
        if (fill_color.a > 0u && fill_color.a < 255u) {
            break;
        }
        ++first_transparent;
    }

    for (uint32_t scan = first_transparent + 1u; scan < count; ++scan) {
        Color fill_color = commands[scan].fill_color;
        if (fill_color.a > 0u && fill_color.a < 255u) {
            continue;
        }

        swap_face_commands(commands[first_transparent], commands[scan]);
        ++first_transparent;
    }

    return first_transparent;
}

static void append_face_command(sdk::Cube_Face_Draw* commands, uint32_t capacity, uint32_t& count,
                                Vector3 center, float edge_length, uint32_t direction,
                                uint32_t edge_mask, const Cube_Visual_Style& style,
                                const Camera3D& camera) {
    if (commands == 0 || count >= capacity) {
        return;
    }

    sdk::Cube_Face_Draw& face = commands[count];
    face = sdk::Cube_Face_Draw{};
    face.center = center;
    face.edge_length = edge_length;
    face.sort_depth = face_view_depth(camera, center, edge_length, direction);
    face.fill_color = style.fill_color;
    face.edge_color = style.edge_color;
    face.direction = (uint8_t)direction;
    face.edge_mask = (uint8_t)edge_mask;
    ++count;
}

static void append_transparent_cube_faces(sdk::Cube_Face_Draw* commands, uint32_t capacity,
                                          uint32_t& count, Vector3 center, float edge_length,
                                          const Cube_Visual_Style& style, const Camera3D& camera) {
    for (uint32_t direction = sdk::FACE_DIRECTION_POSITIVE_X; direction < sdk::FACE_DIRECTION_COUNT;
         ++direction) {
        uint32_t edge_mask = 0u;
        if (direction == sdk::FACE_DIRECTION_POSITIVE_X ||
            direction == sdk::FACE_DIRECTION_NEGATIVE_X) {
            edge_mask = sdk::FACE_EDGE_ALL;
        }
        if (direction == sdk::FACE_DIRECTION_POSITIVE_Y ||
            direction == sdk::FACE_DIRECTION_NEGATIVE_Y) {
            edge_mask = sdk::FACE_EDGE_TOP | sdk::FACE_EDGE_BOTTOM;
        }

        append_face_command(commands, capacity, count, center, edge_length, direction, edge_mask,
                            style, camera);
    }
}

Application_Config application_default_config(void) {
    Application_Config config = {};

    config.runtime.screen_width = 1920;
    config.runtime.screen_height = 1080;
    config.runtime.target_fps = 60;
    config.runtime.title = "CPU Software Cube Field";
    config.render_resources.font_path = "assets/fonts/FiraCode-Regular.ttf";
    config.render_resources.font_pixel_size = 48;
    config.clear_color = Color{18, 18, 24, 255};

    config.cube_field.dimensions = Grid_Dimensions{500u, 1000u, 100u};
    config.cube_field.cube_size = 1.0f;
    config.cube_field.spacing = 5.0f;
    config.cube_view.focused_radius = 5u;
    config.cube_view.face_text_radius = 3u;
    config.cube_view.birds_eye_sample_stride = 10u;

    config.local.clip_near = 0.01;
    config.local.clip_far = 8000.0;
    config.local.start_distance = 12.0f;
    config.local.orbit.mouse_sensitivity = 0.0035f;
    config.local.orbit.wheel_speed = 1.4f;
    config.local.orbit.minimum_pitch = -1.48f;
    config.local.orbit.maximum_pitch = 1.48f;
    config.local.orbit.minimum_distance = 2.0f;
    config.local.orbit.maximum_distance = 80.0f;
    config.local.orbit.vertical_fov = 60.0f;

    config.birds_eye.clip_near = 1.0;
    config.birds_eye.clip_far = 40000.0;
    config.birds_eye.start_distance = 7200.0f;
    config.birds_eye.start_pitch = 1.30f;
    config.birds_eye.orbit.mouse_sensitivity = 0.0030f;
    config.birds_eye.orbit.wheel_speed = 140.0f;
    config.birds_eye.orbit.minimum_pitch = -1.48f;
    config.birds_eye.orbit.maximum_pitch = 1.48f;
    config.birds_eye.orbit.minimum_distance = 600.0f;
    config.birds_eye.orbit.maximum_distance = 30000.0f;
    config.birds_eye.orbit.vertical_fov = 60.0f;

    config.face_text.font_size = 0.140f;
    config.face_text.glyph_spacing = 0.004f;
    config.face_text.line_spacing = 0.040f;
    config.face_text.surface_offset = 0.012f;
    config.face_text.color = WHITE;

    config.compass.local_arrow_length = 2.65f;
    config.compass.local_shaft_radius = 0.035f;
    config.compass.local_arrowhead_length = 0.65f;
    config.compass.local_arrowhead_radius = 0.12f;
    config.compass.local_label_size = 0.34f;
    config.compass.local_gap_center = 0.95f;
    config.compass.local_gap_half_length = 0.55f;
    config.compass.local_surface_offset = 0.25f;
    config.compass.birds_eye_arrow_length_scale = 0.22f;
    config.compass.birds_eye_shaft_radius_scale = 0.0035f;
    config.compass.birds_eye_arrowhead_length_scale = 0.055f;
    config.compass.birds_eye_arrowhead_radius_scale = 0.010f;
    config.compass.birds_eye_gap_half_length_scale = 0.070f;
    config.compass.birds_eye_surface_offset_scale = 0.015f;
    config.compass.birds_eye_label_pixels = 72.0f;
    config.compass.axis_color[sdk::AXIS_X] = BLUE;
    config.compass.axis_color[sdk::AXIS_Y] = RED;
    config.compass.axis_color[sdk::AXIS_Z] = GREEN;
    config.compass.label_color = WHITE;
    config.compass.label_background_color = BLACK;

    config.boundary_grid.fine_radius = 5u;
    config.boundary_grid.minimum_major_stride = 20u;
    config.boundary_grid.line_color = Color{130, 130, 130, 90};
    config.boundary_grid.border_color = Color{210, 210, 210, 150};

    return config;
}

Application_Result application_memory_requirements(Application_Memory_Requirements& requirements,
                                                   const Application_Config& config) {
    requirements = Application_Memory_Requirements{};

    uint64_t local_side = 2u * (uint64_t)config.cube_view.focused_radius + 1u;
    uint64_t local_x = minimum_u64(local_side, config.cube_field.dimensions.x);
    uint64_t local_y = minimum_u64(local_side, config.cube_field.dimensions.y);
    uint64_t local_z = minimum_u64(local_side, config.cube_field.dimensions.z);
    uint64_t local_cube_count = 0u;
    uint64_t local_face_count = 0u;
    if (!checked_multiply_u64(local_x, local_y, local_cube_count)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }
    if (!checked_multiply_u64(local_cube_count, local_z, local_cube_count)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }
    if (!checked_multiply_u64(local_cube_count, 6u, local_face_count)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }

    uint64_t sample_x = cube_view_sample_count(config.cube_field.dimensions.x,
                                               config.cube_view.birds_eye_sample_stride);
    uint64_t sample_y = cube_view_sample_count(config.cube_field.dimensions.y,
                                               config.cube_view.birds_eye_sample_stride);
    uint64_t sample_z = cube_view_sample_count(config.cube_field.dimensions.z,
                                               config.cube_view.birds_eye_sample_stride);
    uint64_t sample_face_xy = 0u;
    uint64_t sample_face_xz = 0u;
    uint64_t sample_face_yz = 0u;
    if (!checked_multiply_u64(sample_x, sample_y, sample_face_xy)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }
    if (!checked_multiply_u64(sample_x, sample_z, sample_face_xz)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }
    if (!checked_multiply_u64(sample_y, sample_z, sample_face_yz)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }
    if (sample_face_xy > UINT64_MAX - sample_face_xz ||
        sample_face_xy + sample_face_xz > UINT64_MAX - sample_face_yz) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }

    uint64_t birds_eye_face_count = 0u;
    if (!checked_multiply_u64(2u, sample_face_xy + sample_face_xz + sample_face_yz,
                              birds_eye_face_count)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }

    uint64_t face_command_count = local_face_count;
    if (birds_eye_face_count > face_command_count) {
        face_command_count = birds_eye_face_count;
    }
    if (face_command_count > UINT32_MAX) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }

    uint64_t command_bytes = 0u;
    if (!checked_multiply_u64(face_command_count, sizeof(sdk::Cube_Face_Draw), command_bytes)) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }
    if (command_bytes > SIZE_MAX) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }

    requirements.temporary.capacity = (size_t)command_bytes;
    requirements.temporary.alignment = FACE_COMMAND_ALIGNMENT;
    requirements.maximum_face_command_count = (uint32_t)face_command_count;
    return APPLICATION_RESULT_OK;
}

static void switch_to_local_view(Application& application, Cube_Handle selected_cube) {
    if (cube_field_contains_handle(application.field, selected_cube)) {
        application.active_cube = selected_cube;
    }

    application.view_mode = VIEW_MODE_LOCAL;
    sdk::orbit_camera_retarget(application.orbit,
                               cube_field_cube_center(application.field, application.active_cube));
    if (application.orbit.distance < application.config.local.orbit.minimum_distance ||
        application.orbit.distance > application.config.local.orbit.maximum_distance) {
        application.orbit.distance = application.config.local.start_distance;
    }
    application.cursor.wants_capture = 1u;
}

static void switch_to_birds_eye_view(Application& application) {
    application.view_mode = VIEW_MODE_BIRDS_EYE;
    sdk::orbit_camera_retarget(application.orbit, cube_field_center(application.field));
    application.orbit.distance = application.config.birds_eye.start_distance;
    application.orbit.pitch = application.config.birds_eye.start_pitch;
    application.cursor.wants_capture = 1u;
}

Application_Result application_init(Application& application, const Application_Config& config,
                                    alloc::Allocator& persistent_allocator) {
    application = Application{};
    if (!alloc::allocator_is_valid(persistent_allocator)) {
        return APPLICATION_RESULT_INVALID_ALLOCATOR;
    }

    Application_Memory_Requirements memory_requirements = {};
    Application_Result memory_result = application_memory_requirements(memory_requirements, config);
    if (memory_result != APPLICATION_RESULT_OK) {
        return memory_result;
    }

    application.config = config;
    if (!cube_field_initialize(application.field, config.cube_field)) {
        return APPLICATION_RESULT_CUBE_FIELD_INITIALIZATION_FAILED;
    }
    if (sdk::runtime_init(application.runtime, config.runtime)) {
        return APPLICATION_RESULT_RUNTIME_INITIALIZATION_FAILED;
    }
    if (!cube_data_initialize(application.data, application.field, persistent_allocator)) {
        return APPLICATION_RESULT_CUBE_DATA_INITIALIZATION_FAILED;
    }

    cube_palette_set_initialize(application.palettes);
    if (sdk::render_resources_init(application.render_resources, config.render_resources)) {
        return APPLICATION_RESULT_RENDER_RESOURCE_INITIALIZATION_FAILED;
    }

    Grid_Coordinate center_coordinate = {};
    center_coordinate.x = application.field.dimensions.x / 2u;
    center_coordinate.y = application.field.dimensions.y / 2u;
    center_coordinate.z = application.field.dimensions.z / 2u;
    application.active_cube = cube_field_handle(application.field, center_coordinate);
    sdk::orbit_camera_init(
        application.orbit, cube_field_cube_center(application.field, application.active_cube),
        0.75f, 0.45f, config.local.start_distance, config.local.orbit.vertical_fov);
    application.view_mode = VIEW_MODE_LOCAL;
    application.cursor.wants_capture = 1u;
    sdk::cursor_capture_update(application.cursor, 1, IsWindowFocused());
    application.camera = sdk::orbit_camera_build(application.orbit);
    application.initialized = 1u;
    return APPLICATION_RESULT_OK;
}

static Application_Input poll_application_input(void) {
    Application_Input input = {};
    input.mouse_position = GetMousePosition();
    input.mouse_delta = GetMouseDelta();
    input.mouse_wheel = GetMouseWheelMove();
    if (IsWindowFocused()) {
        input.window_focused = 1u;
    }
    if (IsKeyPressed(KEY_G)) {
        input.toggle_view = 1u;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        input.toggle_cursor = 1u;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        input.select_cube = 1u;
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        input.navigate_left = 1u;
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        input.navigate_right = 1u;
    }
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        input.navigate_forward = 1u;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        input.navigate_backward = 1u;
    }
    if (IsKeyPressed(KEY_ONE)) {
        input.palette_1 = 1u;
    }
    if (IsKeyPressed(KEY_TWO)) {
        input.palette_2 = 1u;
    }
    if (IsKeyPressed(KEY_THREE)) {
        input.palette_3 = 1u;
    }

    return input;
}

static const sdk::Orbit_Config& active_orbit_config(const Application& application) {
    if (application.view_mode == VIEW_MODE_LOCAL) {
        return application.config.local.orbit;
    }

    return application.config.birds_eye.orbit;
}

static void navigate_active_cube(Application& application, Vector3 camera_direction) {
    if (!cube_field_contains_handle(application.field, application.active_cube)) {
        return;
    }

    float absolute_x = fabsf(camera_direction.x);
    float absolute_y = fabsf(camera_direction.y);
    float absolute_z = fabsf(camera_direction.z);
    int delta_x = 0;
    int delta_y = 0;
    int delta_z = 0;

    if (absolute_y >= absolute_x && absolute_y >= absolute_z) {
        delta_y = -1;
        if (camera_direction.y >= 0.0f) {
            delta_y = 1;
        }
    } else if (absolute_x >= absolute_z) {
        delta_x = -1;
        if (camera_direction.x >= 0.0f) {
            delta_x = 1;
        }
    } else {
        delta_z = -1;
        if (camera_direction.z >= 0.0f) {
            delta_z = 1;
        }
    }

    Grid_Coordinate current = cube_field_coordinate(application.field, application.active_cube);
    Grid_Coordinate next = {};
    next.x =
        (uint32_t)clamp_int((int)current.x + delta_x, 0, (int)application.field.dimensions.x - 1);
    next.y =
        (uint32_t)clamp_int((int)current.y + delta_y, 0, (int)application.field.dimensions.y - 1);
    next.z =
        (uint32_t)clamp_int((int)current.z + delta_z, 0, (int)application.field.dimensions.z - 1);
    Cube_Handle next_cube = cube_field_handle(application.field, next);
    if (next_cube != CUBE_HANDLE_NONE) {
        switch_to_local_view(application, next_cube);
    }
}

static uint32_t boundary_grid_stride(uint32_t dimension, uint32_t minimum_major_stride) {
    uint32_t stride = dimension / 40u;
    if (stride < minimum_major_stride) {
        stride = minimum_major_stride;
    }
    if (stride == 0u) {
        stride = 1u;
    }

    return stride;
}

static int boundary_grid_index_visible(uint32_t index, uint32_t focus, uint32_t dimension,
                                       uint32_t fine_radius, uint32_t major_stride) {
    if (index == 0u || index + 1u == dimension) {
        return 1;
    }
    if (index % major_stride == 0u) {
        return 1;
    }

    uint32_t minimum = 0u;
    if (focus > fine_radius) {
        minimum = focus - fine_radius;
    }
    uint64_t maximum = (uint64_t)focus + fine_radius;
    if (index >= minimum && (uint64_t)index <= maximum) {
        return 1;
    }

    return 0;
}

static Color boundary_grid_line_color(uint32_t index, uint32_t dimension,
                                      const Boundary_Grid_Config& config) {
    if (index == 0u || index + 1u == dimension) {
        return config.border_color;
    }

    return config.line_color;
}

static void emit_boundary_grid_line(Vector3 start, Vector3 end, Color color) {
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(start.x, start.y, start.z);
    rlVertex3f(end.x, end.y, end.z);
}

// All six field boundary planes remain visible, but distant lines use a major
// stride while lines near the active coordinate remain dense. One rlgl batch
// avoids hundreds of independent DrawLine3D submissions.
static void draw_local_boundary_grids(const Application& application) {
    const Cube_Field& field = application.field;
    const Boundary_Grid_Config& config = application.config.boundary_grid;
    Grid_Coordinate active = cube_field_coordinate(field, application.active_cube);
    uint32_t stride_x = boundary_grid_stride(field.dimensions.x, config.minimum_major_stride);
    uint32_t stride_y = boundary_grid_stride(field.dimensions.y, config.minimum_major_stride);
    uint32_t stride_z = boundary_grid_stride(field.dimensions.z, config.minimum_major_stride);

    rlSetTexture(0);
    rlBegin(RL_LINES);
    for (uint32_t y = 0u; y < field.dimensions.y; ++y) {
        if (!boundary_grid_index_visible(y, active.y, field.dimensions.y, config.fine_radius,
                                         stride_y)) {
            continue;
        }
        float world_y = field.first_center.y + (float)y * field.spacing;
        Color color = boundary_grid_line_color(y, field.dimensions.y, config);
        emit_boundary_grid_line(Vector3{field.bounds_min.x, world_y, field.bounds_min.z},
                                Vector3{field.bounds_min.x, world_y, field.bounds_max.z}, color);
        emit_boundary_grid_line(Vector3{field.bounds_max.x, world_y, field.bounds_min.z},
                                Vector3{field.bounds_max.x, world_y, field.bounds_max.z}, color);
        emit_boundary_grid_line(Vector3{field.bounds_min.x, world_y, field.bounds_min.z},
                                Vector3{field.bounds_max.x, world_y, field.bounds_min.z}, color);
        emit_boundary_grid_line(Vector3{field.bounds_min.x, world_y, field.bounds_max.z},
                                Vector3{field.bounds_max.x, world_y, field.bounds_max.z}, color);
    }

    for (uint32_t x = 0u; x < field.dimensions.x; ++x) {
        if (!boundary_grid_index_visible(x, active.x, field.dimensions.x, config.fine_radius,
                                         stride_x)) {
            continue;
        }
        float world_x = field.first_center.x + (float)x * field.spacing;
        Color color = boundary_grid_line_color(x, field.dimensions.x, config);
        emit_boundary_grid_line(Vector3{world_x, field.bounds_min.y, field.bounds_min.z},
                                Vector3{world_x, field.bounds_min.y, field.bounds_max.z}, color);
        emit_boundary_grid_line(Vector3{world_x, field.bounds_max.y, field.bounds_min.z},
                                Vector3{world_x, field.bounds_max.y, field.bounds_max.z}, color);
        emit_boundary_grid_line(Vector3{world_x, field.bounds_min.y, field.bounds_min.z},
                                Vector3{world_x, field.bounds_max.y, field.bounds_min.z}, color);
        emit_boundary_grid_line(Vector3{world_x, field.bounds_min.y, field.bounds_max.z},
                                Vector3{world_x, field.bounds_max.y, field.bounds_max.z}, color);
    }

    for (uint32_t z = 0u; z < field.dimensions.z; ++z) {
        if (!boundary_grid_index_visible(z, active.z, field.dimensions.z, config.fine_radius,
                                         stride_z)) {
            continue;
        }
        float world_z = field.first_center.z + (float)z * field.spacing;
        Color color = boundary_grid_line_color(z, field.dimensions.z, config);
        emit_boundary_grid_line(Vector3{field.bounds_min.x, field.bounds_min.y, world_z},
                                Vector3{field.bounds_min.x, field.bounds_max.y, world_z}, color);
        emit_boundary_grid_line(Vector3{field.bounds_max.x, field.bounds_min.y, world_z},
                                Vector3{field.bounds_max.x, field.bounds_max.y, world_z}, color);
        emit_boundary_grid_line(Vector3{field.bounds_min.x, field.bounds_min.y, world_z},
                                Vector3{field.bounds_max.x, field.bounds_min.y, world_z}, color);
        emit_boundary_grid_line(Vector3{field.bounds_min.x, field.bounds_max.y, world_z},
                                Vector3{field.bounds_max.x, field.bounds_max.y, world_z}, color);
    }
    rlEnd();
}

static void draw_cube_face_text(const Application& application, Cube_Handle cube,
                                Vector3 cube_center, uint8_t value, const float* value_widths,
                                const float* direction_widths) {
    char handle_text[32];
    snprintf(handle_text, sizeof(handle_text), "%u", cube);
    const char* lines[3] = {handle_text, cube_value_label(value), 0};
    float measured_widths[3] = {};
    measured_widths[0] = sdk::measure_plane_text_width(application.render_resources, handle_text,
                                                       application.config.face_text.font_size,
                                                       application.config.face_text.glyph_spacing);
    if (value < CUBE_VALUE_COUNT) {
        measured_widths[1] = value_widths[value];
    }

    for (uint32_t direction = sdk::FACE_DIRECTION_POSITIVE_X; direction < sdk::FACE_DIRECTION_COUNT;
         ++direction) {
        const sdk::Axis_Direction_Info& direction_info = sdk::axis_direction_info(direction);
        float surface_distance =
            0.5f * application.field.cube_size + application.config.face_text.surface_offset;
        Vector3 face_center =
            Vector3Add(cube_center, Vector3Scale(direction_info.normal, surface_distance));
        Vector3 face_to_camera = Vector3Subtract(application.camera.position, face_center);
        if (Vector3DotProduct(direction_info.normal, face_to_camera) <= 0.0f) {
            continue;
        }

        lines[2] = direction_info.label;
        measured_widths[2] = direction_widths[direction];
        sdk::Plane_Text_Block_Draw text_draw = {};
        text_draw.lines = lines;
        text_draw.measured_widths = measured_widths;
        text_draw.line_count = 3u;
        text_draw.center = face_center;
        text_draw.right = direction_info.right;
        text_draw.down = direction_info.down;
        text_draw.font_size = application.config.face_text.font_size;
        text_draw.glyph_spacing = application.config.face_text.glyph_spacing;
        text_draw.line_spacing = application.config.face_text.line_spacing;
        text_draw.color = application.config.face_text.color;
        sdk::draw_plane_text_block_immediate(application.render_resources, text_draw);
    }
}

static void draw_local_compass(const Application& application) {
    Vector3 selected_center = cube_field_cube_center(application.field, application.active_cube);
    const Compass_Config& config = application.config.compass;
    for (uint32_t direction = sdk::FACE_DIRECTION_POSITIVE_X; direction < sdk::FACE_DIRECTION_COUNT;
         ++direction) {
        const sdk::Axis_Direction_Info& direction_info = sdk::axis_direction_info(direction);
        float face_distance = 0.5f * application.field.cube_size + config.local_surface_offset;
        Vector3 origin =
            Vector3Add(selected_center, Vector3Scale(direction_info.normal, face_distance));
        float gap_center = config.local_gap_center;

        sdk::Arrow_Draw arrow = {};
        arrow.origin = origin;
        arrow.direction = direction_info.normal;
        arrow.length = config.local_arrow_length;
        arrow.shaft_radius = config.local_shaft_radius;
        arrow.arrowhead_length = config.local_arrowhead_length;
        arrow.arrowhead_radius = config.local_arrowhead_radius;
        arrow.gap_center = gap_center;
        arrow.gap_half_length = config.local_gap_half_length;
        arrow.color = config.axis_color[direction_info.axis];
        sdk::draw_arrow_immediate(arrow);

        sdk::Billboard_Text_Draw label = {};
        label.text = direction_info.label;
        label.center = Vector3Add(origin, Vector3Scale(direction_info.normal, gap_center));
        label.font_size = config.local_label_size;
        label.glyph_spacing = 0.01f;
        label.color = config.label_color;
        sdk::draw_billboard_text_immediate(application.render_resources, application.camera, label);
    }
}

static uint32_t focused_face_command_capacity(const Grid_Region& region) {
    uint64_t count_x = (uint64_t)region.maximum.x - region.minimum.x + 1u;
    uint64_t count_y = (uint64_t)region.maximum.y - region.minimum.y + 1u;
    uint64_t count_z = (uint64_t)region.maximum.z - region.minimum.z + 1u;
    uint64_t face_count = count_x * count_y * count_z * 6u;
    if (face_count > UINT32_MAX) {
        return 0u;
    }

    return (uint32_t)face_count;
}

static Application_Result render_local_view(Application& application,
                                            alloc::Allocator& temporary_allocator) {
    Grid_Coordinate selected = cube_field_coordinate(application.field, application.active_cube);
    Grid_Region region = cube_view_focused_region(application.field, application.active_cube,
                                                  application.config.cube_view.focused_radius);
    uint32_t command_capacity = focused_face_command_capacity(region);
    size_t command_bytes = (size_t)command_capacity * sizeof(sdk::Cube_Face_Draw);
    sdk::Cube_Face_Draw* transparent_faces = (sdk::Cube_Face_Draw*)alloc::allocator_allocate(
        temporary_allocator, command_bytes, FACE_COMMAND_ALIGNMENT);
    if (command_bytes != 0u && transparent_faces == 0) {
        return APPLICATION_RESULT_TEMPORARY_MEMORY_EXHAUSTED;
    }

    const Cube_Palette& palette = cube_palette_set_active(application.palettes);
    uint32_t transparent_count = 0u;
    uint32_t radius = application.config.cube_view.focused_radius;
    draw_local_boundary_grids(application);

    for (uint32_t z = region.minimum.z; z <= region.maximum.z; ++z) {
        for (uint32_t y = region.minimum.y; y <= region.maximum.y; ++y) {
            Grid_Coordinate row_coordinate = {region.minimum.x, y, z};
            Cube_Handle cube = cube_field_handle(application.field, row_coordinate);
            Vector3 cube_center = cube_field_coordinate_center(application.field, row_coordinate);
            for (uint32_t x = region.minimum.x; x <= region.maximum.x; ++x) {
                Grid_Coordinate coordinate = {x, y, z};
                if (cube_view_focused_accepts_coordinate(selected, coordinate, radius)) {
                    uint8_t value = cube_data_value(application.data, cube);
                    const Cube_Visual_Style& style = cube_palette_style(palette, value);
                    if (style.fill_color.a > 0u && style.fill_color.a < 255u) {
                        append_transparent_cube_faces(
                            transparent_faces, command_capacity, transparent_count, cube_center,
                            application.field.cube_size, style, application.camera);
                    } else {
                        sdk::Cube_Draw cube_draw = {};
                        cube_draw.center = cube_center;
                        cube_draw.edge_length = application.field.cube_size;
                        cube_draw.fill_color = style.fill_color;
                        cube_draw.edge_color = style.edge_color;
                        sdk::draw_cube_immediate(cube_draw);
                    }
                }

                ++cube;
                cube_center.x += application.field.spacing;
            }
        }
    }

    sort_faces_far_to_near(transparent_faces, transparent_count);
    sdk::draw_cube_faces_immediate(transparent_faces, transparent_count);

    float value_widths[CUBE_VALUE_COUNT] = {};
    float direction_widths[sdk::FACE_DIRECTION_COUNT] = {};
    for (uint32_t value = CUBE_VALUE_A; value < CUBE_VALUE_COUNT; ++value) {
        value_widths[value] = sdk::measure_plane_text_width(
            application.render_resources, cube_value_label((uint8_t)value),
            application.config.face_text.font_size, application.config.face_text.glyph_spacing);
    }
    for (uint32_t direction = sdk::FACE_DIRECTION_POSITIVE_X; direction < sdk::FACE_DIRECTION_COUNT;
         ++direction) {
        direction_widths[direction] = sdk::measure_plane_text_width(
            application.render_resources, sdk::axis_direction_info(direction).label,
            application.config.face_text.font_size, application.config.face_text.glyph_spacing);
    }

    Grid_Region text_region = cube_view_focused_region(
        application.field, application.active_cube, application.config.cube_view.face_text_radius);
    uint32_t text_radius = application.config.cube_view.face_text_radius;
    for (uint32_t z = text_region.minimum.z; z <= text_region.maximum.z; ++z) {
        for (uint32_t y = text_region.minimum.y; y <= text_region.maximum.y; ++y) {
            Grid_Coordinate row_coordinate = {text_region.minimum.x, y, z};
            Cube_Handle cube = cube_field_handle(application.field, row_coordinate);
            Vector3 cube_center = cube_field_coordinate_center(application.field, row_coordinate);
            for (uint32_t x = text_region.minimum.x; x <= text_region.maximum.x; ++x) {
                Grid_Coordinate coordinate = {x, y, z};
                if (cube_view_focused_accepts_coordinate(selected, coordinate, text_radius)) {
                    draw_cube_face_text(application, cube, cube_center,
                                        cube_data_value(application.data, cube), value_widths,
                                        direction_widths);
                }

                ++cube;
                cube_center.x += application.field.spacing;
            }
        }
    }

    sdk::Cube_Draw selected_cube_draw = {};
    selected_cube_draw.center = cube_field_cube_center(application.field, application.active_cube);
    selected_cube_draw.edge_length = application.field.cube_size;
    selected_cube_draw.edge_color = YELLOW;
    sdk::draw_cube_immediate(selected_cube_draw);
    draw_local_compass(application);

    alloc::allocator_release(temporary_allocator, transparent_faces, command_bytes,
                             FACE_COMMAND_ALIGNMENT);
    return APPLICATION_RESULT_OK;
}

static uint8_t birds_eye_edge_mask(uint32_t direction, Grid_Coordinate sample,
                                   const Birds_Eye_Lattice& lattice) {
    uint32_t face_column = 0u;
    uint32_t face_row = 0u;
    uint32_t face_column_count = 0u;
    uint32_t face_row_count = 0u;
    uint32_t top_neighbor = sdk::FACE_DIRECTION_NONE;
    uint32_t right_neighbor = sdk::FACE_DIRECTION_NONE;
    uint32_t bottom_neighbor = sdk::FACE_DIRECTION_NONE;
    uint32_t left_neighbor = sdk::FACE_DIRECTION_NONE;

    switch (direction) {
    case sdk::FACE_DIRECTION_POSITIVE_X:
        face_column = lattice.sample_count.z - 1u - sample.z;
        face_row = lattice.sample_count.y - 1u - sample.y;
        face_column_count = lattice.sample_count.z;
        face_row_count = lattice.sample_count.y;
        top_neighbor = sdk::FACE_DIRECTION_POSITIVE_Y;
        right_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Z;
        bottom_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Y;
        left_neighbor = sdk::FACE_DIRECTION_POSITIVE_Z;
        break;
    case sdk::FACE_DIRECTION_NEGATIVE_X:
        face_column = sample.z;
        face_row = lattice.sample_count.y - 1u - sample.y;
        face_column_count = lattice.sample_count.z;
        face_row_count = lattice.sample_count.y;
        top_neighbor = sdk::FACE_DIRECTION_POSITIVE_Y;
        right_neighbor = sdk::FACE_DIRECTION_POSITIVE_Z;
        bottom_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Y;
        left_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Z;
        break;
    case sdk::FACE_DIRECTION_POSITIVE_Y:
        face_column = sample.x;
        face_row = sample.z;
        face_column_count = lattice.sample_count.x;
        face_row_count = lattice.sample_count.z;
        top_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Z;
        right_neighbor = sdk::FACE_DIRECTION_POSITIVE_X;
        bottom_neighbor = sdk::FACE_DIRECTION_POSITIVE_Z;
        left_neighbor = sdk::FACE_DIRECTION_NEGATIVE_X;
        break;
    case sdk::FACE_DIRECTION_NEGATIVE_Y:
        face_column = sample.x;
        face_row = lattice.sample_count.z - 1u - sample.z;
        face_column_count = lattice.sample_count.x;
        face_row_count = lattice.sample_count.z;
        top_neighbor = sdk::FACE_DIRECTION_POSITIVE_Z;
        right_neighbor = sdk::FACE_DIRECTION_POSITIVE_X;
        bottom_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Z;
        left_neighbor = sdk::FACE_DIRECTION_NEGATIVE_X;
        break;
    case sdk::FACE_DIRECTION_POSITIVE_Z:
        face_column = sample.x;
        face_row = lattice.sample_count.y - 1u - sample.y;
        face_column_count = lattice.sample_count.x;
        face_row_count = lattice.sample_count.y;
        top_neighbor = sdk::FACE_DIRECTION_POSITIVE_Y;
        right_neighbor = sdk::FACE_DIRECTION_POSITIVE_X;
        bottom_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Y;
        left_neighbor = sdk::FACE_DIRECTION_NEGATIVE_X;
        break;
    case sdk::FACE_DIRECTION_NEGATIVE_Z:
        face_column = lattice.sample_count.x - 1u - sample.x;
        face_row = lattice.sample_count.y - 1u - sample.y;
        face_column_count = lattice.sample_count.x;
        face_row_count = lattice.sample_count.y;
        top_neighbor = sdk::FACE_DIRECTION_POSITIVE_Y;
        right_neighbor = sdk::FACE_DIRECTION_NEGATIVE_X;
        bottom_neighbor = sdk::FACE_DIRECTION_NEGATIVE_Y;
        left_neighbor = sdk::FACE_DIRECTION_POSITIVE_X;
        break;
    default:
        return 0u;
    }

    uint32_t edge_mask = 0u;
    if (face_row > 0u || direction < top_neighbor) {
        edge_mask |= sdk::FACE_EDGE_TOP;
    }
    if (face_column > 0u || direction < left_neighbor) {
        edge_mask |= sdk::FACE_EDGE_LEFT;
    }
    if (face_column_count != 0u && face_column + 1u == face_column_count &&
        direction < right_neighbor) {
        edge_mask |= sdk::FACE_EDGE_RIGHT;
    }
    if (face_row_count != 0u && face_row + 1u == face_row_count && direction < bottom_neighbor) {
        edge_mask |= sdk::FACE_EDGE_BOTTOM;
    }

    return (uint8_t)edge_mask;
}

static void
append_birds_eye_sample_faces(const Application& application, const Cube_Palette& palette,
                              const Birds_Eye_Lattice& lattice, const Birds_Eye_Sample& sample,
                              sdk::Cube_Face_Draw* commands, uint32_t capacity, uint32_t& count) {
    uint8_t value = cube_data_value(application.data, sample.source_cube);
    const Cube_Visual_Style& style = cube_palette_style(palette, value);

    struct Face_Mapping {
        uint32_t flag;
        uint32_t direction;
    };
    static const Face_Mapping FACE_MAPPINGS[6] = {
        {BIRDS_EYE_FACE_NEGATIVE_X, sdk::FACE_DIRECTION_NEGATIVE_X},
        {BIRDS_EYE_FACE_POSITIVE_X, sdk::FACE_DIRECTION_POSITIVE_X},
        {BIRDS_EYE_FACE_NEGATIVE_Y, sdk::FACE_DIRECTION_NEGATIVE_Y},
        {BIRDS_EYE_FACE_POSITIVE_Y, sdk::FACE_DIRECTION_POSITIVE_Y},
        {BIRDS_EYE_FACE_NEGATIVE_Z, sdk::FACE_DIRECTION_NEGATIVE_Z},
        {BIRDS_EYE_FACE_POSITIVE_Z, sdk::FACE_DIRECTION_POSITIVE_Z}};

    for (uint32_t mapping_index = 0u; mapping_index < 6u; ++mapping_index) {
        const Face_Mapping& mapping = FACE_MAPPINGS[mapping_index];
        if ((sample.outward_faces & mapping.flag) == 0u) {
            continue;
        }

        append_face_command(
            commands, capacity, count, sample.center, lattice.representative_size,
            mapping.direction,
            birds_eye_edge_mask(mapping.direction, sample.lattice_coordinate, lattice), style,
            application.camera);
    }
}

static uint32_t birds_eye_face_command_capacity(const Birds_Eye_Lattice& lattice) {
    uint64_t sample_face_xy = (uint64_t)lattice.sample_count.x * lattice.sample_count.y;
    uint64_t sample_face_xz = (uint64_t)lattice.sample_count.x * lattice.sample_count.z;
    uint64_t sample_face_yz = (uint64_t)lattice.sample_count.y * lattice.sample_count.z;
    uint64_t face_count = 2u * (sample_face_xy + sample_face_xz + sample_face_yz);
    if (face_count > UINT32_MAX) {
        return 0u;
    }

    return (uint32_t)face_count;
}

static Application_Result render_birds_eye_shell(Application& application,
                                                 alloc::Allocator& temporary_allocator,
                                                 Birds_Eye_Lattice& lattice) {
    cube_view_build_birds_eye_lattice(lattice, application.field, application.config.cube_view);
    uint32_t command_capacity = birds_eye_face_command_capacity(lattice);
    size_t command_bytes = (size_t)command_capacity * sizeof(sdk::Cube_Face_Draw);
    sdk::Cube_Face_Draw* face_commands = (sdk::Cube_Face_Draw*)alloc::allocator_allocate(
        temporary_allocator, command_bytes, FACE_COMMAND_ALIGNMENT);
    if (command_bytes != 0u && face_commands == 0) {
        return APPLICATION_RESULT_TEMPORARY_MEMORY_EXHAUSTED;
    }

    uint32_t face_count = 0u;
    uint32_t shell_sample_count = cube_view_birds_eye_shell_sample_count(lattice);
    const Cube_Palette& palette = cube_palette_set_active(application.palettes);
    for (uint32_t shell_index = 0u; shell_index < shell_sample_count; ++shell_index) {
        Birds_Eye_Sample sample = {};
        if (!cube_view_birds_eye_shell_sample(sample, application.field, lattice, shell_index)) {
            continue;
        }

        append_birds_eye_sample_faces(application, palette, lattice, sample, face_commands,
                                      command_capacity, face_count);
    }

    uint32_t first_transparent = partition_transparent_faces(face_commands, face_count);
    sort_faces_far_to_near(face_commands + first_transparent, face_count - first_transparent);
    sdk::draw_cube_faces_immediate(face_commands, face_count);
    alloc::allocator_release(temporary_allocator, face_commands, command_bytes,
                             FACE_COMMAND_ALIGNMENT);
    return APPLICATION_RESULT_OK;
}

static Vector3 birds_eye_face_center(const Birds_Eye_Lattice& lattice,
                                     const sdk::Axis_Direction_Info& direction_info) {
    Vector3 face_center = lattice.center;
    if (direction_info.normal.x > 0.0f) {
        face_center.x = lattice.bounds_max.x;
    }
    if (direction_info.normal.x < 0.0f) {
        face_center.x = lattice.bounds_min.x;
    }
    if (direction_info.normal.y > 0.0f) {
        face_center.y = lattice.bounds_max.y;
    }
    if (direction_info.normal.y < 0.0f) {
        face_center.y = lattice.bounds_min.y;
    }
    if (direction_info.normal.z > 0.0f) {
        face_center.z = lattice.bounds_max.z;
    }
    if (direction_info.normal.z < 0.0f) {
        face_center.z = lattice.bounds_min.z;
    }

    return face_center;
}

static void draw_birds_eye_compass_3d(const Application& application,
                                      const Birds_Eye_Lattice& lattice, Birds_Eye_Label* labels) {
    float span_x = lattice.bounds_max.x - lattice.bounds_min.x;
    float span_y = lattice.bounds_max.y - lattice.bounds_min.y;
    float span_z = lattice.bounds_max.z - lattice.bounds_min.z;
    float maximum_span = maximum_of_three(span_x, span_y, span_z);
    const Compass_Config& config = application.config.compass;
    float arrow_length = maximum_span * config.birds_eye_arrow_length_scale;
    float shaft_radius = maximum_span * config.birds_eye_shaft_radius_scale;
    float arrowhead_length = maximum_span * config.birds_eye_arrowhead_length_scale;
    float arrowhead_radius = maximum_span * config.birds_eye_arrowhead_radius_scale;
    float gap_half_length = maximum_span * config.birds_eye_gap_half_length_scale;
    float surface_offset = maximum_span * config.birds_eye_surface_offset_scale;

    for (uint32_t direction = sdk::FACE_DIRECTION_POSITIVE_X; direction < sdk::FACE_DIRECTION_COUNT;
         ++direction) {
        const sdk::Axis_Direction_Info& direction_info = sdk::axis_direction_info(direction);
        Vector3 face_center = birds_eye_face_center(lattice, direction_info);
        Vector3 origin =
            Vector3Add(face_center, Vector3Scale(direction_info.normal, surface_offset));
        float gap_center = 0.5f * arrow_length;

        sdk::Arrow_Draw arrow = {};
        arrow.origin = origin;
        arrow.direction = direction_info.normal;
        arrow.length = arrow_length;
        arrow.shaft_radius = shaft_radius;
        arrow.arrowhead_length = arrowhead_length;
        arrow.arrowhead_radius = arrowhead_radius;
        arrow.gap_center = gap_center;
        arrow.gap_half_length = gap_half_length;
        arrow.color = config.axis_color[direction_info.axis];
        sdk::draw_arrow_immediate(arrow);

        Birds_Eye_Label& label = labels[direction - 1u];
        label.text = direction_info.label;
        label.world_position = Vector3Add(origin, Vector3Scale(direction_info.normal, gap_center));
        Vector3 face_to_camera = Vector3Subtract(application.camera.position, face_center);
        if (Vector3DotProduct(direction_info.normal, face_to_camera) > 0.0f) {
            label.visible = 1u;
        }
    }
}

static void draw_birds_eye_labels_2d(const Application& application,
                                     const Birds_Eye_Label* labels) {
    const Font& font = application.render_resources.face_font;
    float font_size = application.config.compass.birds_eye_label_pixels;
    for (uint32_t label_index = 0u; label_index < 6u; ++label_index) {
        const Birds_Eye_Label& label = labels[label_index];
        if (!label.visible || label.text == 0) {
            continue;
        }

        Vector2 screen_position = GetWorldToScreen(label.world_position, application.camera);
        Vector2 text_size = MeasureTextEx(font, label.text, font_size, 2.0f);
        int panel_x = (int)(screen_position.x - 0.5f * text_size.x - 12.0f);
        int panel_y = (int)(screen_position.y - 0.5f * text_size.y - 8.0f);
        int panel_width = (int)(text_size.x + 24.0f);
        int panel_height = (int)(text_size.y + 16.0f);
        DrawRectangle(panel_x, panel_y, panel_width, panel_height,
                      application.config.compass.label_background_color);
        DrawTextEx(
            font, label.text,
            Vector2{screen_position.x - 0.5f * text_size.x, screen_position.y - 0.5f * text_size.y},
            font_size, 2.0f, application.config.compass.label_color);
    }
}

static void draw_application_overlay(const Application& application) {
    const char* view_name = "birds-eye";
    if (application.view_mode == VIEW_MODE_LOCAL) {
        view_name = "local";
    }

    Grid_Coordinate coordinate = cube_field_coordinate(application.field, application.active_cube);
    const Cube_Palette& palette = cube_palette_set_active(application.palettes);
    int overlay_height = 82;
    if (!application.cursor.wants_capture) {
        overlay_height = 108;
    }

    DrawRectangle(8, 8, 910, overlay_height, Color{0, 0, 0, 180});
    DrawText(TextFormat("FPS %d | %s | palette %u %s | active %u (%u,%u,%u)", GetFPS(), view_name,
                        application.palettes.active_palette, palette.name, application.active_cube,
                        coordinate.x, coordinate.y, coordinate.z),
             16, 16, 20, RAYWHITE);
    DrawText("G: view | RMB: capture/select | LMB while free: target | 1/2/3: palette | local "
             "WASD/arrows: snap",
             16, 42, 18, LIGHTGRAY);
    if (!application.cursor.wants_capture) {
        DrawText("cursor free: rotation paused; click a submitted cube to retarget and recapture",
                 16, 70, 18, YELLOW);
    }
}

static void update_application_state(Application& application, const Application_Input& input) {
    if (input.palette_1) {
        cube_palette_set_select(application.palettes, CUBE_PALETTE_HANDLE_DEFAULT);
    }
    if (input.palette_2) {
        cube_palette_set_select(application.palettes, CUBE_PALETTE_HANDLE_PASTEL);
    }
    if (input.palette_3) {
        cube_palette_set_select(application.palettes, CUBE_PALETTE_HANDLE_WARM);
    }

    if (input.toggle_view) {
        if (application.view_mode == VIEW_MODE_LOCAL) {
            switch_to_birds_eye_view(application);
        } else {
            switch_to_local_view(application, application.active_cube);
        }
    }
    if (input.toggle_cursor) {
        int wants_capture = 1;
        if (application.cursor.wants_capture) {
            wants_capture = 0;
        }
        application.cursor.wants_capture = (uint8_t)wants_capture;
    }

    sdk::cursor_capture_update(application.cursor, application.cursor.wants_capture,
                               input.window_focused);
    Vector2 mouse_delta = input.mouse_delta;
    float mouse_wheel = input.mouse_wheel;
    int rotate_from_mouse = 0;
    if (application.cursor.is_captured && !application.cursor.suppress_mouse_delta) {
        rotate_from_mouse = 1;
    } else {
        mouse_delta = Vector2{};
    }
    if (!application.cursor.is_captured) {
        mouse_wheel = 0.0f;
    }
    application.cursor.suppress_mouse_delta = 0u;

    const sdk::Orbit_Config& orbit_config = active_orbit_config(application);
    sdk::orbit_camera_update(application.orbit, orbit_config, mouse_delta, mouse_wheel,
                             rotate_from_mouse);
    application.camera = sdk::orbit_camera_build(application.orbit);

    if (application.view_mode == VIEW_MODE_LOCAL) {
        Vector3 camera_forward = Vector3Normalize(
            Vector3Subtract(application.camera.target, application.camera.position));
        Vector3 camera_right =
            Vector3Normalize(Vector3CrossProduct(camera_forward, application.camera.up));
        if (input.navigate_forward) {
            navigate_active_cube(application, camera_forward);
        }
        if (input.navigate_backward) {
            navigate_active_cube(application, Vector3Negate(camera_forward));
        }
        if (input.navigate_right) {
            navigate_active_cube(application, camera_right);
        }
        if (input.navigate_left) {
            navigate_active_cube(application, Vector3Negate(camera_right));
        }
        application.camera = sdk::orbit_camera_build(application.orbit);
    }

    if (!application.cursor.wants_capture && input.select_cube && input.window_focused) {
        Ray selection_ray = GetScreenToWorldRay(input.mouse_position, application.camera);
        Cube_Pick_Result pick = {};
        if (application.view_mode == VIEW_MODE_LOCAL) {
            pick =
                cube_view_pick_focused(application.field, application.active_cube,
                                       application.config.cube_view.focused_radius, selection_ray);
        } else {
            Birds_Eye_Lattice lattice = {};
            cube_view_build_birds_eye_lattice(lattice, application.field,
                                              application.config.cube_view);
            pick = cube_view_pick_birds_eye(application.field, lattice, selection_ray);
        }

        if (pick.cube != CUBE_HANDLE_NONE) {
            switch_to_local_view(application, pick.cube);
            sdk::cursor_capture_update(application.cursor, 1, input.window_focused);
            application.camera = sdk::orbit_camera_build(application.orbit);
        }
    }
}

Application_Result application_frame(Application& application,
                                     alloc::Allocator& temporary_allocator) {
    if (!application.initialized) {
        return APPLICATION_RESULT_INVALID_CONFIGURATION;
    }
    if (!alloc::allocator_is_valid(temporary_allocator)) {
        return APPLICATION_RESULT_INVALID_ALLOCATOR;
    }

    Application_Input input = poll_application_input();
    update_application_state(application, input);

    Application_Result render_result = APPLICATION_RESULT_OK;
    Birds_Eye_Label birds_eye_labels[6] = {};
    BeginDrawing();
    ClearBackground(application.config.clear_color);
    if (application.view_mode == VIEW_MODE_LOCAL) {
        rlSetClipPlanes(application.config.local.clip_near, application.config.local.clip_far);
    } else {
        rlSetClipPlanes(application.config.birds_eye.clip_near,
                        application.config.birds_eye.clip_far);
    }

    BeginMode3D(application.camera);
    if (application.view_mode == VIEW_MODE_LOCAL) {
        render_result = render_local_view(application, temporary_allocator);
    } else if (application.view_mode == VIEW_MODE_BIRDS_EYE) {
        Birds_Eye_Lattice lattice = {};
        render_result = render_birds_eye_shell(application, temporary_allocator, lattice);
        if (render_result == APPLICATION_RESULT_OK) {
            draw_birds_eye_compass_3d(application, lattice, birds_eye_labels);
        }
    }
    EndMode3D();

    if (application.view_mode == VIEW_MODE_BIRDS_EYE && render_result == APPLICATION_RESULT_OK) {
        draw_birds_eye_labels_2d(application, birds_eye_labels);
    }
    draw_application_overlay(application);
    EndDrawing();
    return render_result;
}

void application_shutdown(Application& application, alloc::Allocator& persistent_allocator) {
    sdk::cursor_capture_shutdown(application.cursor);
    sdk::render_resources_shutdown(application.render_resources);
    cube_data_shutdown(application.data, persistent_allocator);
    sdk::runtime_shutdown(application.runtime);
    application = Application{};
}

const char* application_result_label(Application_Result result) {
    switch (result) {
    case APPLICATION_RESULT_OK:
        return "ok";
    case APPLICATION_RESULT_INVALID_CONFIGURATION:
        return "invalid configuration";
    case APPLICATION_RESULT_INVALID_ALLOCATOR:
        return "invalid allocator";
    case APPLICATION_RESULT_RUNTIME_INITIALIZATION_FAILED:
        return "runtime initialization failed";
    case APPLICATION_RESULT_CUBE_FIELD_INITIALIZATION_FAILED:
        return "cube field initialization failed";
    case APPLICATION_RESULT_CUBE_DATA_INITIALIZATION_FAILED:
        return "cube data initialization failed";
    case APPLICATION_RESULT_RENDER_RESOURCE_INITIALIZATION_FAILED:
        return "render resource initialization failed";
    case APPLICATION_RESULT_TEMPORARY_MEMORY_EXHAUSTED:
        return "temporary memory exhausted";
    default:
        return "unknown application result";
    }
}

} // namespace app

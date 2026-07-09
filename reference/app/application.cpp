#include "app/application.h"

#include "sdk/rendering.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

namespace app {

static const int LOCAL_RENDER_RADIUS = 5;
static const int LOCAL_TEXT_RADIUS = 3;
static const uint32_t BIRD_BLOCK_STEP = 10;
static const int MAX_BILLBOARD_LABELS = 256;

struct Cube_Draw_Command {
    Vector3 center;
    Vector3 size;
    Color fill;
    Color edge;
    float distance_to_camera_sq;
};

static float square(float x)
{
    return x * x;
}

static float distance_squared(Vector3 a, Vector3 b)
{
    return square(a.x - b.x) + square(a.y - b.y) + square(a.z - b.z);
}

static int compare_cube_commands_far_to_near(const void* a, const void* b)
{
    const Cube_Draw_Command* left = (const Cube_Draw_Command*)a;
    const Cube_Draw_Command* right = (const Cube_Draw_Command*)b;

    if (left->distance_to_camera_sq < right->distance_to_camera_sq) {
        return 1;
    }

    if (left->distance_to_camera_sq > right->distance_to_camera_sq) {
        return -1;
    }

    return 0;
}

static void derive_camera_from_orbit(
    const sdk::Orbit_Camera_Config& config,
    const sdk::Orbit_Camera_State& state,
    Camera3D& camera)
{
    float horizontal_radius = cosf(state.pitch) * state.distance;
    float vertical_offset = sinf(state.pitch) * state.distance;

    camera.target = state.target;
    camera.position.x = state.target.x + sinf(state.yaw) * horizontal_radius;
    camera.position.y = state.target.y + vertical_offset;
    camera.position.z = state.target.z + cosf(state.yaw) * horizontal_radius;
    camera.up = config.up;
    camera.fovy = config.fovy;
    camera.projection = config.projection;
}

static Vector3 vector_for_axis_step(int axis, int sign)
{
    Vector3 result = {};

    if (axis == 0) {
        result.x = (float)sign;
    } else if (axis == 1) {
        result.y = (float)sign;
    } else {
        result.z = (float)sign;
    }

    return result;
}

static void dominant_axis_step(Vector3 direction, int& dx, int& dy, int& dz)
{
    dx = 0;
    dy = 0;
    dz = 0;

    float ax = fabsf(direction.x);
    float ay = fabsf(direction.y);
    float az = fabsf(direction.z);

    if (ax >= ay && ax >= az) {
        dx = direction.x >= 0.0f ? 1 : -1;
    } else if (ay >= ax && ay >= az) {
        dy = direction.y >= 0.0f ? 1 : -1;
    } else {
        dz = direction.z >= 0.0f ? 1 : -1;
    }
}

static void navigate_active_cube(App_State& state, const Camera3D& camera)
{
    if (state.view_mode != APP_VIEW_LOCAL) {
        return;
    }

    int pressed_forward = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
    int pressed_backward = IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);
    int pressed_left = IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT);
    int pressed_right = IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT);

    if (!pressed_forward && !pressed_backward && !pressed_left && !pressed_right) {
        return;
    }

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, Vector3{ 0.0f, 1.0f, 0.0f }));

    if (Vector3LengthSqr(right) < 0.0001f) {
        right = Vector3{ 1.0f, 0.0f, 0.0f };
    }

    Vector3 move = {};

    if (pressed_forward) {
        move = Vector3Add(move, forward);
    }
    if (pressed_backward) {
        move = Vector3Subtract(move, forward);
    }
    if (pressed_right) {
        move = Vector3Add(move, right);
    }
    if (pressed_left) {
        move = Vector3Subtract(move, right);
    }

    int dx = 0;
    int dy = 0;
    int dz = 0;
    dominant_axis_step(move, dx, dy, dz);

    Cube_Coords coords = cube_coords_from_handle(state.config.field, state.active_cube);
    state.active_cube = cube_handle_clamped(
        state.config.field,
        (int)coords.x + dx,
        (int)coords.y + dy,
        (int)coords.z + dz);

    state.local_camera.target = cube_center_from_handle(state.config.field, state.active_cube);
}

static void cube_face_basis(int face, Vector3& normal, Vector3& right, Vector3& up, const char*& face_name)
{
    if (face == 0) {
        normal = Vector3{  1.0f,  0.0f,  0.0f };
        right  = Vector3{  0.0f,  0.0f, -1.0f };
        up     = Vector3{  0.0f,  1.0f,  0.0f };
        face_name = "+x";
    } else if (face == 1) {
        normal = Vector3{ -1.0f,  0.0f,  0.0f };
        right  = Vector3{  0.0f,  0.0f,  1.0f };
        up     = Vector3{  0.0f,  1.0f,  0.0f };
        face_name = "-x";
    } else if (face == 2) {
        normal = Vector3{  0.0f,  1.0f,  0.0f };
        right  = Vector3{  1.0f,  0.0f,  0.0f };
        up     = Vector3{  0.0f,  0.0f, -1.0f };
        face_name = "+y";
    } else if (face == 3) {
        normal = Vector3{  0.0f, -1.0f,  0.0f };
        right  = Vector3{  1.0f,  0.0f,  0.0f };
        up     = Vector3{  0.0f,  0.0f,  1.0f };
        face_name = "-y";
    } else if (face == 4) {
        normal = Vector3{  0.0f,  0.0f,  1.0f };
        right  = Vector3{  1.0f,  0.0f,  0.0f };
        up     = Vector3{  0.0f,  1.0f,  0.0f };
        face_name = "+z";
    } else {
        normal = Vector3{  0.0f,  0.0f, -1.0f };
        right  = Vector3{ -1.0f,  0.0f,  0.0f };
        up     = Vector3{  0.0f,  1.0f,  0.0f };
        face_name = "-z";
    }
}

static void draw_cube_face_text(
    const Cube_Field& field,
    Cube_Handle handle,
    Cube_Value value,
    Vector3 center)
{
    float face_offset = field.cube_size * 0.5f + 0.0125f;
    float font_size = field.cube_size * 0.115f;
    float spacing = field.cube_size * 0.012f;
    float line_spacing = field.cube_size * 0.145f;

    char index_text[64] = {};
    snprintf(index_text, sizeof(index_text), "#%u", handle);

    for (int face = 0; face < 6; ++face) {
        Vector3 normal = {};
        Vector3 right = {};
        Vector3 up = {};
        const char* face_name = "";
        cube_face_basis(face, normal, right, up, face_name);

        Vector3 face_center = Vector3Add(center, Vector3Scale(normal, face_offset));
        sdk::draw_text_on_plane_3d(index_text, Vector3Add(face_center, Vector3Scale(up, line_spacing)), right, up, font_size, spacing, WHITE);
        sdk::draw_text_on_plane_3d(cube_value_name(value), face_center, right, up, font_size, spacing, WHITE);
        sdk::draw_text_on_plane_3d(face_name, Vector3Subtract(face_center, Vector3Scale(up, line_spacing)), right, up, font_size, spacing, WHITE);
    }
}

static void render_local_compass(App_State& state, sdk::Billboard_Label_Buffer& labels)
{
    Vector3 center = cube_center_from_handle(state.config.field, state.active_cube);
    float face_offset = state.config.field.cube_size * 0.5f + 0.05f;
    float length = state.config.field.spacing * 0.65f;
    float thickness = 0.065f;
    float gap = 0.72f;

    sdk::draw_axis_compass_arrow(labels, Vector3Add(center, Vector3Scale(vector_for_axis_step(0,  1), face_offset)), 0,  1, length, thickness, gap, BLUE,  "+x");
    sdk::draw_axis_compass_arrow(labels, Vector3Add(center, Vector3Scale(vector_for_axis_step(0, -1), face_offset)), 0, -1, length, thickness, gap, BLUE,  "-x");
    sdk::draw_axis_compass_arrow(labels, Vector3Add(center, Vector3Scale(vector_for_axis_step(1,  1), face_offset)), 1,  1, length, thickness, gap, RED,   "+y");
    sdk::draw_axis_compass_arrow(labels, Vector3Add(center, Vector3Scale(vector_for_axis_step(1, -1), face_offset)), 1, -1, length, thickness, gap, RED,   "-y");
    sdk::draw_axis_compass_arrow(labels, Vector3Add(center, Vector3Scale(vector_for_axis_step(2,  1), face_offset)), 2,  1, length, thickness, gap, GREEN, "+z");
    sdk::draw_axis_compass_arrow(labels, Vector3Add(center, Vector3Scale(vector_for_axis_step(2, -1), face_offset)), 2, -1, length, thickness, gap, GREEN, "-z");
}

static void render_bird_compass(App_State& state, const Camera3D& camera, sdk::Billboard_Label_Buffer& labels)
{
    Vector3 field_min = cube_field_min(state.config.field);
    Vector3 field_max = cube_field_max(state.config.field);
    Vector3 field_size = cube_field_world_size(state.config.field);

    float max_extent = field_size.x;
    if (field_size.y > max_extent) max_extent = field_size.y;
    if (field_size.z > max_extent) max_extent = field_size.z;

    float length = max_extent * 0.18f;
    float thickness = max_extent * 0.004f;
    float label_font_size = max_extent * 0.050f;
    float label_padding = label_font_size * 0.25f;
    float gap = label_font_size * 2.40f;
    float outward = state.config.field.spacing * (float)BIRD_BLOCK_STEP;
    Color label_background = Color{ 0, 0, 0, 255 };

    Vector3 x_face_pos = Vector3{ field_max.x + outward, 0.0f, 0.0f };
    Vector3 x_face_neg = Vector3{ field_min.x - outward, 0.0f, 0.0f };
    Vector3 y_face_pos = Vector3{ 0.0f, field_max.y + outward, 0.0f };
    Vector3 y_face_neg = Vector3{ 0.0f, field_min.y - outward, 0.0f };
    Vector3 z_face_pos = Vector3{ 0.0f, 0.0f, field_max.z + outward };
    Vector3 z_face_neg = Vector3{ 0.0f, 0.0f, field_min.z - outward };

    const char* x_pos_label = camera.position.x >= field_max.x ? "+x" : "";
    const char* x_neg_label = camera.position.x <= field_min.x ? "-x" : "";
    const char* y_pos_label = camera.position.y >= field_max.y ? "+y" : "";
    const char* y_neg_label = camera.position.y <= field_min.y ? "-y" : "";
    const char* z_pos_label = camera.position.z >= field_max.z ? "+z" : "";
    const char* z_neg_label = camera.position.z <= field_min.z ? "-z" : "";

    sdk::draw_axis_compass_arrow_ex(labels, x_face_pos, 0,  1, length, thickness, gap, BLUE,  x_pos_label, label_font_size, WHITE, label_background, label_padding);
    sdk::draw_axis_compass_arrow_ex(labels, x_face_neg, 0, -1, length, thickness, gap, BLUE,  x_neg_label, label_font_size, WHITE, label_background, label_padding);
    sdk::draw_axis_compass_arrow_ex(labels, y_face_pos, 1,  1, length, thickness, gap, RED,   y_pos_label, label_font_size, WHITE, label_background, label_padding);
    sdk::draw_axis_compass_arrow_ex(labels, y_face_neg, 1, -1, length, thickness, gap, RED,   y_neg_label, label_font_size, WHITE, label_background, label_padding);
    sdk::draw_axis_compass_arrow_ex(labels, z_face_pos, 2,  1, length, thickness, gap, GREEN, z_pos_label, label_font_size, WHITE, label_background, label_padding);
    sdk::draw_axis_compass_arrow_ex(labels, z_face_neg, 2, -1, length, thickness, gap, GREEN, z_neg_label, label_font_size, WHITE, label_background, label_padding);
}

static void draw_command(const Camera3D& camera, const Cube_Draw_Command& command)
{
    sdk::draw_cube_software_friendly(camera, command.center, command.size, command.fill, command.edge);
}

static void draw_birds_eye_face(Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color fill)
{
    fill.a = 255;

    rlCheckRenderBatchLimit(6);
    rlSetTexture(0);

    rlBegin(RL_TRIANGLES);
        rlColor4ub(fill.r, fill.g, fill.b, fill.a);
        rlVertex3f(a.x, a.y, a.z);
        rlVertex3f(b.x, b.y, b.z);
        rlVertex3f(c.x, c.y, c.z);

        rlVertex3f(a.x, a.y, a.z);
        rlVertex3f(c.x, c.y, c.z);
        rlVertex3f(d.x, d.y, d.z);
    rlEnd();
}

static void draw_birds_eye_shell_block_faces(
    const Cube_Draw_Command& command,
    uint32_t bx,
    uint32_t by,
    uint32_t bz,
    uint32_t blocks_x,
    uint32_t blocks_y,
    uint32_t blocks_z)
{
    Vector3 half = Vector3Scale(command.size, 0.5f);

    float min_x = command.center.x - half.x;
    float max_x = command.center.x + half.x;
    float min_y = command.center.y - half.y;
    float max_y = command.center.y + half.y;
    float min_z = command.center.z - half.z;
    float max_z = command.center.z + half.z;

    if (bx == blocks_x - 1u) {
        draw_birds_eye_face(
            Vector3{ max_x, min_y, min_z },
            Vector3{ max_x, min_y, max_z },
            Vector3{ max_x, max_y, max_z },
            Vector3{ max_x, max_y, min_z },
            command.fill);
    }

    if (bx == 0) {
        draw_birds_eye_face(
            Vector3{ min_x, min_y, max_z },
            Vector3{ min_x, min_y, min_z },
            Vector3{ min_x, max_y, min_z },
            Vector3{ min_x, max_y, max_z },
            command.fill);
    }

    if (by == blocks_y - 1u) {
        draw_birds_eye_face(
            Vector3{ min_x, max_y, min_z },
            Vector3{ max_x, max_y, min_z },
            Vector3{ max_x, max_y, max_z },
            Vector3{ min_x, max_y, max_z },
            command.fill);
    }

    if (by == 0) {
        draw_birds_eye_face(
            Vector3{ min_x, min_y, max_z },
            Vector3{ max_x, min_y, max_z },
            Vector3{ max_x, min_y, min_z },
            Vector3{ min_x, min_y, min_z },
            command.fill);
    }

    if (bz == blocks_z - 1u) {
        draw_birds_eye_face(
            Vector3{ max_x, min_y, max_z },
            Vector3{ min_x, min_y, max_z },
            Vector3{ min_x, max_y, max_z },
            Vector3{ max_x, max_y, max_z },
            command.fill);
    }

    if (bz == 0) {
        draw_birds_eye_face(
            Vector3{ min_x, min_y, min_z },
            Vector3{ max_x, min_y, min_z },
            Vector3{ max_x, max_y, min_z },
            Vector3{ min_x, max_y, min_z },
            command.fill);
    }
}

static void render_local_view(App_State& state, const Camera3D& camera, allocators::Arena_Allocator& arena, sdk::Billboard_Label_Buffer& labels)
{
    Cube_Coords active = cube_coords_from_handle(state.config.field, state.active_cube);
    int min_x = (int)active.x - LOCAL_RENDER_RADIUS;
    int max_x = (int)active.x + LOCAL_RENDER_RADIUS;
    int min_y = (int)active.y - LOCAL_RENDER_RADIUS;
    int max_y = (int)active.y + LOCAL_RENDER_RADIUS;
    int min_z = (int)active.z - LOCAL_RENDER_RADIUS;
    int max_z = (int)active.z + LOCAL_RENDER_RADIUS;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (min_z < 0) min_z = 0;
    if (max_x >= (int)state.config.field.count_x) max_x = (int)state.config.field.count_x - 1;
    if (max_y >= (int)state.config.field.count_y) max_y = (int)state.config.field.count_y - 1;
    if (max_z >= (int)state.config.field.count_z) max_z = (int)state.config.field.count_z - 1;

    int max_commands = (2*LOCAL_RENDER_RADIUS + 1) * (2*LOCAL_RENDER_RADIUS + 1) * (2*LOCAL_RENDER_RADIUS + 1);
    Cube_Draw_Command* transparent = (Cube_Draw_Command*)allocators::arena_push(arena, sizeof(Cube_Draw_Command) * (size_t)max_commands, alignof(Cube_Draw_Command));
    int transparent_count = 0;

    Vector3 cube_size = Vector3{ state.config.field.cube_size, state.config.field.cube_size, state.config.field.cube_size };

    for (int z = min_z; z <= max_z; ++z) {
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                Cube_Handle handle = cube_handle_from_coords(state.config.field, (uint32_t)x, (uint32_t)y, (uint32_t)z);
                Cube_Value value = cube_value_at(state.cube_data, handle);
                Vector3 center = cube_center(state.config.field, (uint32_t)x, (uint32_t)y, (uint32_t)z);
                Color fill = state.palette.fill[value];
                Color edge = state.palette.edge[value];

                if (fill.a < 255 && transparent && transparent_count < max_commands) {
                    Cube_Draw_Command* command = transparent + transparent_count;
                    command->center = center;
                    command->size = cube_size;
                    command->fill = fill;
                    command->edge = edge;
                    command->distance_to_camera_sq = distance_squared(camera.position, center);
                    ++transparent_count;
                } else {
                    sdk::draw_cube_software_friendly(camera, center, cube_size, fill, edge);
                }

            }
        }
    }

    if (transparent && transparent_count > 1) {
        qsort(transparent, (size_t)transparent_count, sizeof(Cube_Draw_Command), compare_cube_commands_far_to_near);
    }

    for (int i = 0; i < transparent_count; ++i) {
        draw_command(camera, transparent[i]);
    }

    for (int z = min_z; z <= max_z; ++z) {
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                int dx = x - (int)active.x;
                int dy = y - (int)active.y;
                int dz = z - (int)active.z;

                if (dx*dx + dy*dy + dz*dz <= LOCAL_TEXT_RADIUS * LOCAL_TEXT_RADIUS) {
                    Cube_Handle handle = cube_handle_from_coords(state.config.field, (uint32_t)x, (uint32_t)y, (uint32_t)z);
                    Cube_Value value = cube_value_at(state.cube_data, handle);
                    Vector3 center = cube_center(state.config.field, (uint32_t)x, (uint32_t)y, (uint32_t)z);
                    draw_cube_face_text(state.config.field, handle, value, center);
                }
            }
        }
    }

    sdk::draw_boundary_grid_planes(cube_field_min(state.config.field), cube_field_max(state.config.field), state.config.field.spacing, Color{ 80, 80, 80, 125 });
    render_local_compass(state, labels);
}

static uint32_t block_count_for_axis(uint32_t cube_count)
{
    return (cube_count + BIRD_BLOCK_STEP - 1u) / BIRD_BLOCK_STEP;
}

static void block_range(uint32_t block_index, uint32_t cube_count, uint32_t& start, uint32_t& end)
{
    start = block_index * BIRD_BLOCK_STEP;
    end = start + BIRD_BLOCK_STEP;
    if (end > cube_count) {
        end = cube_count;
    }
}

static Vector3 bird_block_center(const Cube_Field& field, uint32_t bx, uint32_t by, uint32_t bz)
{
    uint32_t sx = 0, ex = 0;
    uint32_t sy = 0, ey = 0;
    uint32_t sz = 0, ez = 0;
    block_range(bx, field.count_x, sx, ex);
    block_range(by, field.count_y, sy, ey);
    block_range(bz, field.count_z, sz, ez);

    Vector3 a = cube_center(field, sx, sy, sz);
    Vector3 b = cube_center(field, ex - 1u, ey - 1u, ez - 1u);
    return Vector3Scale(Vector3Add(a, b), 0.5f);
}

static Vector3 bird_block_size(const Cube_Field& field, uint32_t bx, uint32_t by, uint32_t bz)
{
    uint32_t sx = 0, ex = 0;
    uint32_t sy = 0, ey = 0;
    uint32_t sz = 0, ez = 0;
    block_range(bx, field.count_x, sx, ex);
    block_range(by, field.count_y, sy, ey);
    block_range(bz, field.count_z, sz, ez);

    return Vector3{
        field.spacing * (float)(ex - sx),
        field.spacing * (float)(ey - sy),
        field.spacing * (float)(ez - sz)
    };
}

static Cube_Handle bird_block_source_handle(const Cube_Field& field, uint32_t bx, uint32_t by, uint32_t bz)
{
    uint32_t sx = 0, ex = 0;
    uint32_t sy = 0, ey = 0;
    uint32_t sz = 0, ez = 0;
    block_range(bx, field.count_x, sx, ex);
    block_range(by, field.count_y, sy, ey);
    block_range(bz, field.count_z, sz, ez);

    return cube_handle_from_coords(field, (sx + ex - 1u)/2u, (sy + ey - 1u)/2u, (sz + ez - 1u)/2u);
}

static void render_birds_eye_view(App_State& state, const Camera3D& camera, allocators::Arena_Allocator& arena, sdk::Billboard_Label_Buffer& labels)
{
    (void)camera;
    (void)arena;

    uint32_t blocks_x = block_count_for_axis(state.config.field.count_x);
    uint32_t blocks_y = block_count_for_axis(state.config.field.count_y);
    uint32_t blocks_z = block_count_for_axis(state.config.field.count_z);

    rlDisableBackfaceCulling();

    for (uint32_t bz = 0; bz < blocks_z; ++bz) {
        for (uint32_t by = 0; by < blocks_y; ++by) {
            for (uint32_t bx = 0; bx < blocks_x; ++bx) {
                int is_shell =
                    bx == 0 || bx == blocks_x - 1u ||
                    by == 0 || by == blocks_y - 1u ||
                    bz == 0 || bz == blocks_z - 1u;

                if (!is_shell) {
                    continue;
                }

                Cube_Handle handle = bird_block_source_handle(state.config.field, bx, by, bz);
                Cube_Value value = cube_value_at(state.cube_data, handle);
                Color fill = state.palette.fill[value];
                Color edge = state.palette.edge[value];

                Cube_Draw_Command command = {};
                command.center = bird_block_center(state.config.field, bx, by, bz);
                command.size = bird_block_size(state.config.field, bx, by, bz);
                command.fill = fill;
                command.edge = edge;
                command.distance_to_camera_sq = 0.0f;

                draw_birds_eye_shell_block_faces(command, bx, by, bz, blocks_x, blocks_y, blocks_z);
            }
        }
    }

    rlEnableBackfaceCulling();

    render_bird_compass(state, camera, labels);
}

static Cube_Handle pick_local_cube(App_State& state, const Camera3D& camera)
{
    Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
    Cube_Coords active = cube_coords_from_handle(state.config.field, state.active_cube);
    float best_distance = 1000000000000.0f;
    Cube_Handle best_handle = 0;

    int min_x = (int)active.x - LOCAL_RENDER_RADIUS;
    int max_x = (int)active.x + LOCAL_RENDER_RADIUS;
    int min_y = (int)active.y - LOCAL_RENDER_RADIUS;
    int max_y = (int)active.y + LOCAL_RENDER_RADIUS;
    int min_z = (int)active.z - LOCAL_RENDER_RADIUS;
    int max_z = (int)active.z + LOCAL_RENDER_RADIUS;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (min_z < 0) min_z = 0;
    if (max_x >= (int)state.config.field.count_x) max_x = (int)state.config.field.count_x - 1;
    if (max_y >= (int)state.config.field.count_y) max_y = (int)state.config.field.count_y - 1;
    if (max_z >= (int)state.config.field.count_z) max_z = (int)state.config.field.count_z - 1;

    for (int z = min_z; z <= max_z; ++z) {
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                RayCollision hit = GetRayCollisionBox(ray, cube_bounds(state.config.field, (uint32_t)x, (uint32_t)y, (uint32_t)z));
                if (hit.hit && hit.distance < best_distance) {
                    best_distance = hit.distance;
                    best_handle = cube_handle_from_coords(state.config.field, (uint32_t)x, (uint32_t)y, (uint32_t)z);
                }
            }
        }
    }

    return best_handle;
}

static Cube_Handle pick_bird_cube(App_State& state, const Camera3D& camera)
{
    Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
    uint32_t blocks_x = block_count_for_axis(state.config.field.count_x);
    uint32_t blocks_y = block_count_for_axis(state.config.field.count_y);
    uint32_t blocks_z = block_count_for_axis(state.config.field.count_z);

    float best_distance = 1000000000000.0f;
    Cube_Handle best_handle = 0;

    for (uint32_t bz = 0; bz < blocks_z; ++bz) {
        for (uint32_t by = 0; by < blocks_y; ++by) {
            for (uint32_t bx = 0; bx < blocks_x; ++bx) {
                int is_shell =
                    bx == 0 || bx == blocks_x - 1u ||
                    by == 0 || by == blocks_y - 1u ||
                    bz == 0 || bz == blocks_z - 1u;

                if (!is_shell) {
                    continue;
                }

                Vector3 center = bird_block_center(state.config.field, bx, by, bz);
                Vector3 size = bird_block_size(state.config.field, bx, by, bz);
                Vector3 half = Vector3Scale(size, 0.5f);

                BoundingBox box = {};
                box.min = Vector3Subtract(center, half);
                box.max = Vector3Add(center, half);

                RayCollision hit = GetRayCollisionBox(ray, box);
                if (hit.hit && hit.distance < best_distance) {
                    best_distance = hit.distance;
                    best_handle = bird_block_source_handle(state.config.field, bx, by, bz);
                }
            }
        }
    }

    return best_handle;
}

static void handle_selection(App_State& state, const Camera3D& camera)
{
    sdk::Orbit_Camera_State& orbit = state.view_mode == APP_VIEW_LOCAL ? state.local_camera : state.bird_camera;

    if (orbit.wants_cursor_captured) {
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    Cube_Handle picked = 0;

    if (state.view_mode == APP_VIEW_LOCAL) {
        picked = pick_local_cube(state, camera);
    } else {
        picked = pick_bird_cube(state, camera);
    }

    if (picked != 0) {
        state.active_cube = picked;
        state.view_mode = APP_VIEW_LOCAL;
        state.local_camera.target = cube_center_from_handle(state.config.field, state.active_cube);
        sdk::orbit_camera_request_capture(state.local_camera, 1);
        state.local_camera.suppress_mouse_delta = 1;
    }
}

static void toggle_view_if_requested(App_State& state)
{
    if (!IsKeyPressed(KEY_G)) {
        return;
    }

    if (state.view_mode == APP_VIEW_LOCAL) {
        state.view_mode = APP_VIEW_BIRDS_EYE;
        state.bird_camera.target = cube_field_center(state.config.field);
        sdk::orbit_camera_request_capture(state.bird_camera, 1);
        state.bird_camera.suppress_mouse_delta = 1;
    } else {
        state.view_mode = APP_VIEW_LOCAL;
        state.local_camera.target = cube_center_from_handle(state.config.field, state.active_cube);
        sdk::orbit_camera_request_capture(state.local_camera, 1);
        state.local_camera.suppress_mouse_delta = 1;
    }
}

static void update_palette_from_keyboard(App_State& state)
{
    if (IsKeyPressed(KEY_ONE)) {
        state.active_palette_index = 1u;
        cube_palette_variant(state.palette, state.active_palette_index);
    } else if (IsKeyPressed(KEY_TWO)) {
        state.active_palette_index = 2u;
        cube_palette_variant(state.palette, state.active_palette_index);
    } else if (IsKeyPressed(KEY_THREE)) {
        state.active_palette_index = 3u;
        cube_palette_variant(state.palette, state.active_palette_index);
    }
}

void app_default_config(App_Config& config)
{
    config = {};
    config.runtime.title = "Software Cube Field Reference";
    config.runtime.screen_width = 1920;
    config.runtime.screen_height = 1080;
    config.runtime.target_fps = 60;
    config.runtime.exit_key = KEY_ESCAPE;

    cube_field_init(config.field, 500, 1000, 100, 1.0f, 5.0f);
    config.data_seed = 0x1234abcdu;
    config.frame_arena_bytes = 8u * 1024u * 1024u;
}

App_Result app_init(
    allocators::Allocator& persistent_allocator,
    App_Config& config,
    App_State& state)
{
    state = {};
    state.config = config;

    if (state.config.field.total_count == 0) {
        cube_field_init(state.config.field, 500, 1000, 100, 1.0f, 5.0f);
    }

    if (state.config.frame_arena_bytes == 0) {
        state.config.frame_arena_bytes = 8u * 1024u * 1024u;
    }

    if (sdk::runtime_init(state.runtime, state.config.runtime) != sdk::RUNTIME_SUCCESS) {
        return APP_ERROR_RUNTIME_INIT;
    }

    if (cube_data_create(persistent_allocator, state.config.field, state.cube_data) != CUBE_DATA_SUCCESS) {
        sdk::runtime_shutdown(state.runtime);
        return APP_ERROR_CUBE_DATA;
    }

    cube_data_generate(state.config.field, state.cube_data, state.config.data_seed);
    state.active_palette_index = 1u;
    cube_palette_variant(state.palette, state.active_palette_index);

    state.frame_memory = allocators::allocator_reserve(persistent_allocator, state.config.frame_arena_bytes, 64);
    if (!state.frame_memory) {
        cube_data_destroy(persistent_allocator, state.cube_data);
        sdk::runtime_shutdown(state.runtime);
        return APP_ERROR_FRAME_ARENA_ALLOCATION;
    }

    state.frame_memory_size = state.config.frame_arena_bytes;
    allocators::arena_init(state.frame_arena, state.frame_memory, state.frame_memory_size);

    state.active_cube = cube_handle_from_coords(
        state.config.field,
        state.config.field.count_x / 2u,
        state.config.field.count_y / 2u,
        state.config.field.count_z / 2u);

    state.local_camera_config.radians_per_mouse_pixel = 0.0045f;
    state.local_camera_config.world_units_per_wheel_step = 0.7f;
    state.local_camera_config.minimum_pitch = -85.0f * DEG2RAD;
    state.local_camera_config.maximum_pitch = 85.0f * DEG2RAD;
    state.local_camera_config.minimum_distance = 3.0f;
    state.local_camera_config.maximum_distance = 80.0f;
    state.local_camera_config.up = Vector3{ 0.0f, 1.0f, 0.0f };
    state.local_camera_config.fovy = 70.0f;
    state.local_camera_config.projection = CAMERA_PERSPECTIVE;

    state.local_camera.target = cube_center_from_handle(state.config.field, state.active_cube);
    state.local_camera.yaw = 0.0f;
    state.local_camera.pitch = 30.0f * DEG2RAD;
    state.local_camera.distance = 22.0f;
    state.local_camera.wants_cursor_captured = 1;
    state.local_camera.suppress_mouse_delta = 1;

    Vector3 field_size = cube_field_world_size(state.config.field);
    float largest_extent = field_size.x;
    if (field_size.y > largest_extent) largest_extent = field_size.y;
    if (field_size.z > largest_extent) largest_extent = field_size.z;

    state.bird_camera_config.radians_per_mouse_pixel = 0.0035f;
    state.bird_camera_config.world_units_per_wheel_step = largest_extent * 0.025f;
    state.bird_camera_config.minimum_pitch = -85.0f * DEG2RAD;
    state.bird_camera_config.maximum_pitch = 85.0f * DEG2RAD;
    state.bird_camera_config.minimum_distance = largest_extent * 0.25f;
    state.bird_camera_config.maximum_distance = largest_extent * 4.0f;
    state.bird_camera_config.up = Vector3{ 0.0f, 1.0f, 0.0f };
    state.bird_camera_config.fovy = 70.0f;
    state.bird_camera_config.projection = CAMERA_PERSPECTIVE;

    state.bird_camera.target = cube_field_center(state.config.field);
    state.bird_camera.yaw = 45.0f * DEG2RAD;
    state.bird_camera.pitch = 70.0f * DEG2RAD;
    state.bird_camera.distance = largest_extent * 1.5f;
    state.bird_camera.wants_cursor_captured = 1;
    state.bird_camera.suppress_mouse_delta = 1;

    state.view_mode = APP_VIEW_LOCAL;
    state.is_initialized = 1;
    return APP_SUCCESS;
}

void app_shutdown(
    allocators::Allocator& persistent_allocator,
    App_State& state)
{
    if (!state.is_initialized) {
        return;
    }

    if (state.frame_memory) {
        allocators::allocator_release(persistent_allocator, state.frame_memory);
        state.frame_memory = 0;
        state.frame_memory_size = 0;
    }

    cube_data_destroy(persistent_allocator, state.cube_data);
    sdk::runtime_shutdown(state.runtime);
    state.is_initialized = 0;
}

void app_update_and_render(App_State& state)
{
    allocators::arena_reset(state.frame_arena);

    toggle_view_if_requested(state);
    update_palette_from_keyboard(state);

    sdk::Orbit_Camera_State& orbit = state.view_mode == APP_VIEW_LOCAL ? state.local_camera : state.bird_camera;
    sdk::Orbit_Camera_Config& orbit_config = state.view_mode == APP_VIEW_LOCAL ? state.local_camera_config : state.bird_camera_config;

    if (state.view_mode == APP_VIEW_LOCAL) {
        orbit.target = cube_center_from_handle(state.config.field, state.active_cube);
    } else {
        orbit.target = cube_field_center(state.config.field);
    }

    sdk::Orbit_Camera_Input camera_input = {};
    camera_input.mouse_delta = GetMouseDelta();
    camera_input.wheel_delta = GetMouseWheelMove();
    camera_input.capture_toggle_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    camera_input.is_window_focused = IsWindowFocused();

    Camera3D camera = {};
    sdk::orbit_camera_update(orbit_config, camera_input, orbit, camera);

    navigate_active_cube(state, camera);
    if (state.view_mode == APP_VIEW_LOCAL) {
        orbit.target = cube_center_from_handle(state.config.field, state.active_cube);
        derive_camera_from_orbit(orbit_config, orbit, camera);
    }

    handle_selection(state, camera);
    if (state.view_mode == APP_VIEW_LOCAL) {
        state.local_camera.target = cube_center_from_handle(state.config.field, state.active_cube);
        derive_camera_from_orbit(state.local_camera_config, state.local_camera, camera);
    }

    sdk::Billboard_Label* label_memory = (sdk::Billboard_Label*)allocators::arena_push(
        state.frame_arena,
        sizeof(sdk::Billboard_Label) * (size_t)MAX_BILLBOARD_LABELS,
        alignof(sdk::Billboard_Label));

    sdk::Billboard_Label_Buffer labels = {};
    labels.labels = label_memory;
    labels.capacity = label_memory ? MAX_BILLBOARD_LABELS : 0;
    labels.count = 0;

    Vector3 field_size = cube_field_world_size(state.config.field);
    float largest_extent = field_size.x;
    if (field_size.y > largest_extent) largest_extent = field_size.y;
    if (field_size.z > largest_extent) largest_extent = field_size.z;

    double near_plane = 0.25;
    double far_plane = (double)largest_extent * 4.0;

    if (state.view_mode == APP_VIEW_BIRDS_EYE) {
        near_plane = (double)largest_extent * 0.01;
        if (near_plane < 25.0) {
            near_plane = 25.0;
        }

        far_plane = (double)state.bird_camera.distance + (double)largest_extent * 2.5;
        if (far_plane < (double)largest_extent * 4.0) {
            far_plane = (double)largest_extent * 4.0;
        }
    }

    BeginDrawing();
        ClearBackground(BLACK);

        rlSetClipPlanes(near_plane, far_plane);
        BeginMode3D(camera);
            if (state.view_mode == APP_VIEW_LOCAL) {
                render_local_view(state, camera, state.frame_arena, labels);
                sdk::draw_billboard_labels_3d(camera, labels);
            } else {
                render_birds_eye_view(state, camera, state.frame_arena, labels);
            }
        EndMode3D();

        if (state.view_mode == APP_VIEW_BIRDS_EYE) {
            sdk::draw_billboard_labels_screen_overlay(camera, labels);
        }

        int y = 6;
        DrawText(TextFormat("FPS: %i", GetFPS()), 6, y, 18, YELLOW);
        y += 20;
        DrawText(state.view_mode == APP_VIEW_LOCAL ? "Mode: LOCAL (G -> birds-eye)" : "Mode: BIRDS-EYE (G -> local)", 6, y, 18, RAYWHITE);
        y += 20;
        DrawText(TextFormat("Active cube handle: %u", state.active_cube), 6, y, 18, RAYWHITE);
        y += 20;
        DrawText(TextFormat("Palette: %u (1/2/3)", state.active_palette_index), 6, y, 18, RAYWHITE);
        y += 20;
        DrawText(orbit.wants_cursor_captured ? "Mouse: captured | Right click to select" : "Mouse: free | Left click cube to target", 6, y, 18, RAYWHITE);
        y += 20;
        DrawText("WASD/arrows snap in local view relative to camera angle", 6, y, 18, RAYWHITE);

        if (state.runtime.used_defaults) {
            DrawText("Runtime used default config values", 6, state.config.runtime.screen_height - 28, 18, YELLOW);
        }
    EndDrawing();
}

} // namespace app

#include "app/application.h"

#include "sdk/rendering.h"
#include "rlgl.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

namespace app {

struct Local_Draw_Command {
    Vector3 min;
    Vector3 max;
    Color fill;
    Color edge;
    float distance2;
};

struct Axis_Label {
    const char* text;
    Vector3 position;
    int visible;
};

static Vector3 v3(float x, float y, float z) { return Vector3{x, y, z}; }
static Vector3 add(Vector3 a, Vector3 b) { return v3(a.x+b.x, a.y+b.y, a.z+b.z); }
static Vector3 sub(Vector3 a, Vector3 b) { return v3(a.x-b.x, a.y-b.y, a.z-b.z); }
static Vector3 mul(Vector3 a, float s) { return v3(a.x*s, a.y*s, a.z*s); }
static float dot(Vector3 a, Vector3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static Vector3 cross(Vector3 a, Vector3 b) { return v3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); }
static float len2(Vector3 a) { return dot(a, a); }
static float len(Vector3 a) { return sqrtf(len2(a)); }
static Vector3 norm(Vector3 a) { float l = len(a); return l > 0.0f ? mul(a, 1.0f/l) : v3(0, 0, 0); }
static uint32_t min_u32(uint32_t a, uint32_t b) { return a < b ? a : b; }
static uint32_t max_u32(uint32_t a, uint32_t b) { return a > b ? a : b; }
static int clamp_i32(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static float max3(float a, float b, float c) { return a > b ? (a > c ? a : c) : (b > c ? b : c); }
static int key_pressed2(int a, int b) { return IsKeyPressed(a) || IsKeyPressed(b); }

static int compare_draw_distance_desc(const void* a, const void* b)
{
    float da = ((const Local_Draw_Command*)a)->distance2;
    float db = ((const Local_Draw_Command*)b)->distance2;
    return da < db ? 1 : (da > db ? -1 : 0);
}

static void draw_command_single(const Local_Draw_Command& cmd)
{
    sdk::draw_box_faces(cmd.min, cmd.max, sdk::FACE_ALL, cmd.fill);
    sdk::draw_box_edges(cmd.min, cmd.max, cmd.edge);
}

Application_Config application_default_config(void)
{
    Application_Config c = {};
    c.runtime.screen_width = 1920; c.runtime.screen_height = 1080; c.runtime.target_fps = 60; c.runtime.title = "CPU Software Cube Field";
    c.field_x = 500; c.field_y = 1000; c.field_z = 100; c.cube_size = 1.0f; c.cube_spacing = 5.0f; c.frame_arena_size = 8u*1024u*1024u;
    c.local.render_radius = 5; c.local.text_radius = 3; c.local.grid_fine_radius = 5; c.local.grid_min_major_stride = 20; c.local.clip_near = 0.01f; c.local.clip_far = 8000.0f;
    c.local.orbit.mouse_sensitivity = 0.0035f; c.local.orbit.wheel_speed = 1.4f; c.local.orbit.min_pitch = -1.48f; c.local.orbit.max_pitch = 1.48f; c.local.orbit.min_distance = 2.0f; c.local.orbit.max_distance = 80.0f; c.local.orbit.fovy = 60.0f;
    c.birds_eye.block_source_span = 10; c.birds_eye.clip_near = 1.0f; c.birds_eye.clip_far = 40000.0f; c.birds_eye.start_distance = 7200.0f;
    c.birds_eye.orbit.mouse_sensitivity = 0.0030f; c.birds_eye.orbit.wheel_speed = 140.0f; c.birds_eye.orbit.min_pitch = -1.48f; c.birds_eye.orbit.max_pitch = 1.48f; c.birds_eye.orbit.min_distance = 600.0f; c.birds_eye.orbit.max_distance = 30000.0f; c.birds_eye.orbit.fovy = 60.0f;
    c.compass.local_length = 3.4f; c.compass.local_radius = 0.035f; c.compass.local_label_size = 0.34f; c.compass.bird_length_scale = 0.16f; c.compass.bird_radius_scale = 0.0035f; c.compass.bird_label_pixels = 64.0f;
    return c;
}

static Vector3 field_center(const Cube_Field& f)
{
    return mul(add(f.bounds_min, f.bounds_max), 0.5f);
}

static float field_max_span(const Cube_Field& f)
{
    return max3(f.bounds_max.x - f.bounds_min.x, f.bounds_max.y - f.bounds_min.y, f.bounds_max.z - f.bounds_min.z);
}

static void switch_to_local(Application& app, Cube_Handle target)
{
    app.view_mode = VIEW_LOCAL;
    app.active_cube = target ? target : app.active_cube;
    Cube_Coords c = cube_coords_from_handle(app.field, app.active_cube);
    sdk::orbit_camera_retarget(app.orbit, cube_center(app.field, c.x, c.y, c.z));
    app.orbit.distance = app.orbit.distance < app.config.local.orbit.max_distance ? app.orbit.distance : 12.0f;
    app.cursor.wants_capture = 1;
}

static void switch_to_birds_eye(Application& app)
{
    app.view_mode = VIEW_BIRDS_EYE;
    app.orbit.target = field_center(app.field);
    app.orbit.distance = app.config.birds_eye.start_distance;
    app.orbit.pitch = 1.30f;
    app.cursor.wants_capture = 1;
}

int application_init(Application& app, const Application_Config& config, alloc::Allocator& allocator)
{
    app.config = config;
    app.persistent_allocator = allocator;
    if (sdk::runtime_init(app.runtime, config.runtime)) return 1;

    app.frame_memory_size = config.frame_arena_size;
    app.frame_memory = alloc::allocator_alloc(allocator, app.frame_memory_size, 64);
    if (!app.frame_memory) return 2;
    alloc::arena_init(app.frame_arena, app.frame_memory, app.frame_memory_size);

    cube_field_init(app.field, config.field_x, config.field_y, config.field_z, config.cube_size, config.cube_spacing);
    if (cube_data_generate(app.data, app.field, allocator)) return 3;
    cube_palettes_init(app.palettes);

    uint32_t ax = app.field.count_x/2u, ay = app.field.count_y/2u, az = app.field.count_z/2u;
    app.active_cube = cube_handle_from_coords(app.field, ax, ay, az);
    sdk::orbit_camera_init(app.orbit, cube_center(app.field, ax, ay, az), 0.75f, 0.45f, 12.0f, config.local.orbit.fovy);
    app.cursor.wants_capture = 1;
    app.palette_index = 0;
    app.view_mode = VIEW_LOCAL;

    app.font = LoadFontEx("assets/fonts/FiraCode-Regular.ttf", 48, 0, 0);
    if (app.font.texture.id) app.owns_font = 1; else app.font = GetFontDefault();
    return 0;
}

static void navigate_active_cube(Application& app, Vector3 direction)
{
    Cube_Coords c = cube_coords_from_handle(app.field, app.active_cube);
    float ax = fabsf(direction.x), ay = fabsf(direction.y), az = fabsf(direction.z);
    int dx = 0, dy = 0, dz = 0;
    if (ay >= ax && ay >= az) dy = direction.y >= 0.0f ? 1 : -1;
    else if (ax >= az) dx = direction.x >= 0.0f ? 1 : -1;
    else dz = direction.z >= 0.0f ? 1 : -1;
    uint32_t nx = (uint32_t)clamp_i32((int)c.x + dx, 0, (int)app.field.count_x - 1);
    uint32_t ny = (uint32_t)clamp_i32((int)c.y + dy, 0, (int)app.field.count_y - 1);
    uint32_t nz = (uint32_t)clamp_i32((int)c.z + dz, 0, (int)app.field.count_z - 1);
    Cube_Handle h = cube_handle_from_coords(app.field, nx, ny, nz);
    if (h) switch_to_local(app, h);
}

static void handle_keyboard(Application& app)
{
    if (IsKeyPressed(KEY_ONE)) app.palette_index = 0;
    if (IsKeyPressed(KEY_TWO)) app.palette_index = 1;
    if (IsKeyPressed(KEY_THREE)) app.palette_index = 2;
    if (IsKeyPressed(KEY_G)) { if (app.view_mode == VIEW_LOCAL) switch_to_birds_eye(app); else switch_to_local(app, app.active_cube); }
    if (app.view_mode != VIEW_LOCAL) return;

    Vector3 forward = norm(sub(app.orbit.camera.target, app.orbit.camera.position));
    Vector3 right = norm(cross(forward, app.orbit.camera.up));
    if (key_pressed2(KEY_W, KEY_UP)) navigate_active_cube(app, forward);
    if (key_pressed2(KEY_S, KEY_DOWN)) navigate_active_cube(app, mul(forward, -1.0f));
    if (key_pressed2(KEY_D, KEY_RIGHT)) navigate_active_cube(app, right);
    if (key_pressed2(KEY_A, KEY_LEFT)) navigate_active_cube(app, mul(right, -1.0f));
}

static void draw_text_stack(Font font, const char* a, const char* b, const char* c, Vector3 center, Vector3 right, Vector3 up)
{
    sdk::Text3D_Style s = {};
    s.font_size_world = 0.140f; s.spacing_world = 0.004f; s.text_color = WHITE;
    const char* lines[3] = { a, b, c };
    sdk::draw_text_lines_3d(font, lines, 3, center, right, up, s, 0.180f);
}

static int face_text_visible(const Camera3D& camera, Vector3 face_center, Vector3 normal)
{
    return dot(normal, sub(camera.position, face_center)) > 0.0f;
}

static void draw_cube_face_text(Application& app, Cube_Handle h)
{
    Vector3 mn, mx; cube_bounds(app.field, h, mn, mx);
    Vector3 mid = mul(add(mn, mx), 0.5f);
    char id[32]; snprintf(id, sizeof(id), "%u", h);
    const char* value = cube_value_name(cube_value_at(app.data, h));
    float e = 0.012f;
    Vector3 px = v3(mx.x+e, mid.y, mid.z), nx = v3(mn.x-e, mid.y, mid.z), py = v3(mid.x, mx.y+e, mid.z);
    Vector3 ny = v3(mid.x, mn.y-e, mid.z), pz = v3(mid.x, mid.y, mx.z+e), nz = v3(mid.x, mid.y, mn.z-e);
    if (face_text_visible(app.orbit.camera, px, v3( 1,0,0))) draw_text_stack(app.font, id, value, "+x", px, v3(0,0,-1), v3(0,1,0));
    if (face_text_visible(app.orbit.camera, nx, v3(-1,0,0))) draw_text_stack(app.font, id, value, "-x", nx, v3(0,0, 1), v3(0,1,0));
    if (face_text_visible(app.orbit.camera, py, v3(0, 1,0))) draw_text_stack(app.font, id, value, "+y", py, v3(1,0,0), v3(0,0,-1));
    if (face_text_visible(app.orbit.camera, ny, v3(0,-1,0))) draw_text_stack(app.font, id, value, "-y", ny, v3(1,0,0), v3(0,0, 1));
    if (face_text_visible(app.orbit.camera, pz, v3(0,0, 1))) draw_text_stack(app.font, id, value, "+z", pz, v3(1,0,0), v3(0,1,0));
    if (face_text_visible(app.orbit.camera, nz, v3(0,0,-1))) draw_text_stack(app.font, id, value, "-z", nz, v3(-1,0,0), v3(0,1,0));
}

static int grid_stride(uint32_t count, int min_major_stride)
{
    uint32_t stride = count/40u;
    if (stride < (uint32_t)min_major_stride) stride = (uint32_t)min_major_stride;
    return (int)(stride ? stride : 1u);
}

static int grid_index_visible(uint32_t i, uint32_t focus, uint32_t count, int fine_radius, int stride)
{
    int d = (int)i - (int)focus;
    return i == 0u || i+1u == count || (d >= -fine_radius && d <= fine_radius) || (i%stride) == 0u;
}

static void draw_local_grid_planes(Application& app)
{
    const Cube_Field& f = app.field;
    Cube_Coords active = cube_coords_from_handle(f, app.active_cube);
    Color c = Color{130, 130, 130, 90};
    int radius = app.config.local.grid_fine_radius, sx = grid_stride(f.count_x, app.config.local.grid_min_major_stride);
    int sy = grid_stride(f.count_y, app.config.local.grid_min_major_stride), sz = grid_stride(f.count_z, app.config.local.grid_min_major_stride);
    for (uint32_t y = 0; y < f.count_y; ++y) if (grid_index_visible(y, active.y, f.count_y, radius, sy)) {
        float yy = f.first_center.y + (float)y*f.spacing;
        DrawLine3D(v3(f.bounds_min.x, yy, f.bounds_min.z), v3(f.bounds_min.x, yy, f.bounds_max.z), c);
        DrawLine3D(v3(f.bounds_max.x, yy, f.bounds_min.z), v3(f.bounds_max.x, yy, f.bounds_max.z), c);
    }
    for (uint32_t z = 0; z < f.count_z; ++z) if (grid_index_visible(z, active.z, f.count_z, radius, sz)) {
        float zz = f.first_center.z + (float)z*f.spacing;
        DrawLine3D(v3(f.bounds_min.x, f.bounds_min.y, zz), v3(f.bounds_min.x, f.bounds_max.y, zz), c);
        DrawLine3D(v3(f.bounds_max.x, f.bounds_min.y, zz), v3(f.bounds_max.x, f.bounds_max.y, zz), c);
    }
    for (uint32_t x = 0; x < f.count_x; ++x) if (grid_index_visible(x, active.x, f.count_x, radius, sx)) {
        float xx = f.first_center.x + (float)x*f.spacing;
        DrawLine3D(v3(xx, f.bounds_min.y, f.bounds_min.z), v3(xx, f.bounds_min.y, f.bounds_max.z), c);
        DrawLine3D(v3(xx, f.bounds_max.y, f.bounds_min.z), v3(xx, f.bounds_max.y, f.bounds_max.z), c);
    }
    for (uint32_t z = 0; z < f.count_z; ++z) if (grid_index_visible(z, active.z, f.count_z, radius, sz)) {
        float zz = f.first_center.z + (float)z*f.spacing;
        DrawLine3D(v3(f.bounds_min.x, f.bounds_min.y, zz), v3(f.bounds_max.x, f.bounds_min.y, zz), c);
        DrawLine3D(v3(f.bounds_min.x, f.bounds_max.y, zz), v3(f.bounds_max.x, f.bounds_max.y, zz), c);
    }
    for (uint32_t x = 0; x < f.count_x; ++x) if (grid_index_visible(x, active.x, f.count_x, radius, sx)) {
        float xx = f.first_center.x + (float)x*f.spacing;
        DrawLine3D(v3(xx, f.bounds_min.y, f.bounds_min.z), v3(xx, f.bounds_max.y, f.bounds_min.z), c);
        DrawLine3D(v3(xx, f.bounds_min.y, f.bounds_max.z), v3(xx, f.bounds_max.y, f.bounds_max.z), c);
    }
    for (uint32_t y = 0; y < f.count_y; ++y) if (grid_index_visible(y, active.y, f.count_y, radius, sy)) {
        float yy = f.first_center.y + (float)y*f.spacing;
        DrawLine3D(v3(f.bounds_min.x, yy, f.bounds_min.z), v3(f.bounds_max.x, yy, f.bounds_min.z), c);
        DrawLine3D(v3(f.bounds_min.x, yy, f.bounds_max.z), v3(f.bounds_max.x, yy, f.bounds_max.z), c);
    }
}

static void draw_local_compass(Application& app)
{
    Vector3 o = app.orbit.target;
    float start = app.field.cube_size*0.75f;
    const char* labels[6] = {"+x", "-x", "+y", "-y", "+z", "-z"};
    Vector3 dirs[6] = {v3(1,0,0), v3(-1,0,0), v3(0,1,0), v3(0,-1,0), v3(0,0,1), v3(0,0,-1)};
    Color colors[6] = {BLUE, BLUE, RED, RED, GREEN, GREEN};
    sdk::Text3D_Style style = {};
    style.font_size_world = app.config.compass.local_label_size; style.spacing_world = 0.01f; style.text_color = WHITE;
    for (int i = 0; i < 6; ++i) {
        sdk::draw_arrow_with_gap(o, dirs[i], start, app.config.compass.local_length, 1.25f, app.config.compass.local_radius, colors[i]);
        sdk::draw_billboard_text_3d(app.orbit.camera, app.font, labels[i], add(o, mul(dirs[i], app.config.compass.local_length*0.5f)), style);
    }
}

static void render_local_view(Application& app)
{
    const Cube_Palette& pal = cube_palette(app.palettes, app.palette_index);
    Cube_Coords a = cube_coords_from_handle(app.field, app.active_cube);
    int r = app.config.local.render_radius, tr = app.config.local.text_radius;
    int minx = clamp_i32((int)a.x-r, 0, (int)app.field.count_x-1), maxx = clamp_i32((int)a.x+r, 0, (int)app.field.count_x-1);
    int miny = clamp_i32((int)a.y-r, 0, (int)app.field.count_y-1), maxy = clamp_i32((int)a.y+r, 0, (int)app.field.count_y-1);
    int minz = clamp_i32((int)a.z-r, 0, (int)app.field.count_z-1), maxz = clamp_i32((int)a.z+r, 0, (int)app.field.count_z-1);
    uint32_t max_transparent = (uint32_t)((maxx-minx+1)*(maxy-miny+1)*(maxz-minz+1));
    Local_Draw_Command* transparent = (Local_Draw_Command*)alloc::arena_push(app.frame_arena, sizeof(Local_Draw_Command)*max_transparent, 64);
    uint32_t transparent_count = 0;
    float half = app.field.cube_size*0.5f;

    draw_local_grid_planes(app);
    for (int z = minz; z <= maxz; ++z) for (int y = miny; y <= maxy; ++y) for (int x = minx; x <= maxx; ++x) {
        Cube_Handle h = (Cube_Handle)((uint32_t)z*app.field.stride_z + (uint32_t)y*app.field.stride_y + (uint32_t)x + CUBE_HANDLE_FIRST);
        Cube_Value v = (Cube_Value)app.data.values[h];
        Vector3 center = v3(app.field.first_center.x + (float)x*app.field.spacing, app.field.first_center.y + (float)y*app.field.spacing, app.field.first_center.z + (float)z*app.field.spacing);
        Vector3 mn = v3(center.x - half, center.y - half, center.z - half), mx = v3(center.x + half, center.y + half, center.z + half);
        Local_Draw_Command cmd = {mn, mx, pal.fill[v], pal.edge[v], len2(sub(center, app.orbit.camera.position))};
        if (cmd.fill.a < 255 && transparent) transparent[transparent_count++] = cmd; else draw_command_single(cmd);
    }

    if (transparent && transparent_count) {
        qsort(transparent, transparent_count, sizeof(Local_Draw_Command), compare_draw_distance_desc);
        BeginBlendMode(BLEND_ALPHA);
        for (uint32_t i = 0; i < transparent_count; ++i) draw_command_single(transparent[i]);
        EndBlendMode();
    }

    int tminx = clamp_i32((int)a.x-tr, 0, (int)app.field.count_x-1), tmaxx = clamp_i32((int)a.x+tr, 0, (int)app.field.count_x-1);
    int tminy = clamp_i32((int)a.y-tr, 0, (int)app.field.count_y-1), tmaxy = clamp_i32((int)a.y+tr, 0, (int)app.field.count_y-1);
    int tminz = clamp_i32((int)a.z-tr, 0, (int)app.field.count_z-1), tmaxz = clamp_i32((int)a.z+tr, 0, (int)app.field.count_z-1);
    for (int z = tminz; z <= tmaxz; ++z) for (int y = tminy; y <= tmaxy; ++y) for (int x = tminx; x <= tmaxx; ++x) {
        draw_cube_face_text(app, cube_handle_from_coords(app.field, (uint32_t)x, (uint32_t)y, (uint32_t)z));
    }

    draw_local_compass(app);
}

static uint32_t coarse_count(uint32_t count, uint32_t span)
{
    if (!span) span = 1u;
    return (count + span - 1u)/span;
}

static void coarse_bounds_axis(float mn, float mx, uint32_t i, uint32_t n, float& out_min, float& out_max)
{
    float step = (mx - mn)/(float)n;
    out_min = mn + (float)i*step;
    out_max = i + 1u == n ? mx : out_min + step;
}

static Cube_Handle coarse_source_handle(const Cube_Field& f, uint32_t bx, uint32_t by, uint32_t bz, uint32_t cx, uint32_t cy, uint32_t cz)
{
    uint32_t x0 = (uint32_t)(((uint64_t)bx*f.count_x)/cx), x1 = (uint32_t)(((uint64_t)(bx+1u)*f.count_x)/cx);
    uint32_t y0 = (uint32_t)(((uint64_t)by*f.count_y)/cy), y1 = (uint32_t)(((uint64_t)(by+1u)*f.count_y)/cy);
    uint32_t z0 = (uint32_t)(((uint64_t)bz*f.count_z)/cz), z1 = (uint32_t)(((uint64_t)(bz+1u)*f.count_z)/cz);
    return cube_handle_from_coords(f, min_u32((x0 + max_u32(x1, x0+1u) - 1u)/2u, f.count_x-1u), min_u32((y0 + max_u32(y1, y0+1u) - 1u)/2u, f.count_y-1u), min_u32((z0 + max_u32(z1, z0+1u) - 1u)/2u, f.count_z-1u));
}

static void coarse_block_bounds(const Cube_Field& f, uint32_t bx, uint32_t by, uint32_t bz, uint32_t cx, uint32_t cy, uint32_t cz, Vector3& mn, Vector3& mx)
{
    coarse_bounds_axis(f.bounds_min.x, f.bounds_max.x, bx, cx, mn.x, mx.x);
    coarse_bounds_axis(f.bounds_min.y, f.bounds_max.y, by, cy, mn.y, mx.y);
    coarse_bounds_axis(f.bounds_min.z, f.bounds_max.z, bz, cz, mn.z, mx.z);
}

static void render_birds_eye_shell(Application& app)
{
    const Cube_Palette& pal = cube_palette(app.palettes, app.palette_index);
    uint32_t cx = coarse_count(app.field.count_x, app.config.birds_eye.block_source_span);
    uint32_t cy = coarse_count(app.field.count_y, app.config.birds_eye.block_source_span);
    uint32_t cz = coarse_count(app.field.count_z, app.config.birds_eye.block_source_span);
    BeginBlendMode(BLEND_ALPHA);
    for (uint32_t z = 0; z < cz; ++z) for (uint32_t y = 0; y < cy; ++y) for (uint32_t x = 0; x < cx; ++x) {
        if (x && y && z && x+1u < cx && y+1u < cy && z+1u < cz) continue;
        Vector3 mn, mx; coarse_block_bounds(app.field, x, y, z, cx, cy, cz, mn, mx);
        Cube_Value v = cube_value_at(app.data, coarse_source_handle(app.field, x, y, z, cx, cy, cz));
        unsigned int mask = 0;
        if (x == 0) mask |= sdk::FACE_NEG_X; if (x+1u == cx) mask |= sdk::FACE_POS_X;
        if (y == 0) mask |= sdk::FACE_NEG_Y; if (y+1u == cy) mask |= sdk::FACE_POS_Y;
        if (z == 0) mask |= sdk::FACE_NEG_Z; if (z+1u == cz) mask |= sdk::FACE_POS_Z;
        sdk::draw_box_faces(mn, mx, mask, pal.fill[v]);
    }
    EndBlendMode();
}

static void draw_birds_eye_compass_3d(Application& app, Axis_Label labels[6])
{
    const char* names[6] = {"+x", "-x", "+y", "-y", "+z", "-z"};
    Vector3 dirs[6] = {v3(1,0,0), v3(-1,0,0), v3(0,1,0), v3(0,-1,0), v3(0,0,1), v3(0,0,-1)};
    Color colors[6] = {BLUE, BLUE, RED, RED, GREEN, GREEN};
    Vector3 center = field_center(app.field);
    float max_span = field_max_span(app.field);
    float length = max_span*app.config.compass.bird_length_scale;
    float radius = max_span*app.config.compass.bird_radius_scale;
    Vector3 face[6] = {v3(app.field.bounds_max.x, center.y, center.z), v3(app.field.bounds_min.x, center.y, center.z), v3(center.x, app.field.bounds_max.y, center.z), v3(center.x, app.field.bounds_min.y, center.z), v3(center.x, center.y, app.field.bounds_max.z), v3(center.x, center.y, app.field.bounds_min.z)};

    for (int i = 0; i < 6; ++i) {
        float start = max_span*0.015f;
        sdk::draw_arrow_with_gap(face[i], dirs[i], start, length, length*0.28f, radius, colors[i]);
        labels[i].text = names[i];
        labels[i].position = add(face[i], mul(dirs[i], length*0.5f));
        labels[i].visible = (i == 0 && app.orbit.camera.position.x >= app.field.bounds_max.x) || (i == 1 && app.orbit.camera.position.x <= app.field.bounds_min.x) ||
                            (i == 2 && app.orbit.camera.position.y >= app.field.bounds_max.y) || (i == 3 && app.orbit.camera.position.y <= app.field.bounds_min.y) ||
                            (i == 4 && app.orbit.camera.position.z >= app.field.bounds_max.z) || (i == 5 && app.orbit.camera.position.z <= app.field.bounds_min.z);
    }
}

static void draw_birds_eye_compass_labels_2d(Application& app, Axis_Label labels[6])
{
    for (int i = 0; i < 6; ++i) if (labels[i].visible) {
        Vector2 p = GetWorldToScreen(labels[i].position, app.orbit.camera);
        float size = app.config.compass.bird_label_pixels;
        Vector2 m = MeasureTextEx(app.font, labels[i].text, size, 2.0f);
        DrawRectangle((int)(p.x - m.x*0.5f - 10.0f), (int)(p.y - m.y*0.5f - 6.0f), (int)(m.x + 20.0f), (int)(m.y + 12.0f), BLACK);
        DrawTextEx(app.font, labels[i].text, Vector2{p.x - m.x*0.5f, p.y - m.y*0.5f}, size, 2.0f, WHITE);
    }
}

static Cube_Handle pick_local_cube(Application& app, Ray ray)
{
    Cube_Coords a = cube_coords_from_handle(app.field, app.active_cube);
    int r = app.config.local.render_radius;
    int minx = clamp_i32((int)a.x-r, 0, (int)app.field.count_x-1), maxx = clamp_i32((int)a.x+r, 0, (int)app.field.count_x-1);
    int miny = clamp_i32((int)a.y-r, 0, (int)app.field.count_y-1), maxy = clamp_i32((int)a.y+r, 0, (int)app.field.count_y-1);
    int minz = clamp_i32((int)a.z-r, 0, (int)app.field.count_z-1), maxz = clamp_i32((int)a.z+r, 0, (int)app.field.count_z-1);
    Cube_Handle best = 0; float best_dist = 3.4e38f;
    float half = app.field.cube_size*0.5f;
    for (int z = minz; z <= maxz; ++z) for (int y = miny; y <= maxy; ++y) for (int x = minx; x <= maxx; ++x) {
        Vector3 center = v3(app.field.first_center.x + (float)x*app.field.spacing, app.field.first_center.y + (float)y*app.field.spacing, app.field.first_center.z + (float)z*app.field.spacing);
        Vector3 mn = v3(center.x - half, center.y - half, center.z - half), mx = v3(center.x + half, center.y + half, center.z + half);
        RayCollision hit = GetRayCollisionBox(ray, BoundingBox{mn, mx});
        if (hit.hit && hit.distance < best_dist) { best_dist = hit.distance; best = (Cube_Handle)((uint32_t)z*app.field.stride_z + (uint32_t)y*app.field.stride_y + (uint32_t)x + CUBE_HANDLE_FIRST); }
    }
    return best;
}

static Cube_Handle pick_birds_eye_cube(Application& app, Ray ray)
{
    uint32_t cx = coarse_count(app.field.count_x, app.config.birds_eye.block_source_span);
    uint32_t cy = coarse_count(app.field.count_y, app.config.birds_eye.block_source_span);
    uint32_t cz = coarse_count(app.field.count_z, app.config.birds_eye.block_source_span);
    Cube_Handle best = 0; float best_dist = 3.4e38f; Vector3 best_point = {};
    for (uint32_t z = 0; z < cz; ++z) for (uint32_t y = 0; y < cy; ++y) for (uint32_t x = 0; x < cx; ++x) {
        if (x && y && z && x+1u < cx && y+1u < cy && z+1u < cz) continue;
        Vector3 mn, mx; coarse_block_bounds(app.field, x, y, z, cx, cy, cz, mn, mx);
        RayCollision hit = GetRayCollisionBox(ray, BoundingBox{mn, mx});
        if (hit.hit && hit.distance < best_dist) { best_dist = hit.distance; best_point = hit.point; best = 1; }
    }
    return best ? cube_handle_from_world(app.field, best_point) : 0;
}

static void handle_selection(Application& app)
{
    if (!app.cursor.wants_capture && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Ray ray = GetScreenToWorldRay(GetMousePosition(), app.orbit.camera);
        Cube_Handle h = app.view_mode == VIEW_LOCAL ? pick_local_cube(app, ray) : pick_birds_eye_cube(app, ray);
        if (h) switch_to_local(app, h);
    }
}

static void draw_overlay(Application& app)
{
    Cube_Coords c = cube_coords_from_handle(app.field, app.active_cube);
    const Cube_Palette& pal = cube_palette(app.palettes, app.palette_index);
    DrawRectangle(8, 8, 840, app.cursor.wants_capture ? 82 : 108, Color{0,0,0,170});
    DrawText(TextFormat("FPS %d | %s | palette %u/%u %s | active %u (%u,%u,%u)", GetFPS(), app.view_mode == VIEW_LOCAL ? "local" : "birds-eye", app.palette_index+1u, app.palettes.count, pal.name, app.active_cube, c.x, c.y, c.z), 16, 16, 20, RAYWHITE);
    DrawText("G: view  |  RMB: capture/select  |  LMB while free: target cube  |  1/2/3: palette  |  local WASD/arrows: snap", 16, 42, 18, LIGHTGRAY);
    if (!app.cursor.wants_capture) DrawText("cursor free: camera rotation paused; click a cube to retarget, or right-click to resume rotation", 16, 70, 18, YELLOW);
}

void application_frame(Application& app)
{
    handle_keyboard(app);
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) app.cursor.wants_capture = !app.cursor.wants_capture;

    int focused = IsWindowFocused() ? 1 : 0;
    sdk::cursor_capture_update(app.cursor, app.cursor.wants_capture, focused);
    Vector2 mouse_delta = GetMouseDelta();
    if (!app.cursor.is_captured || app.cursor.suppress_delta) mouse_delta = Vector2{0.0f, 0.0f};
    float wheel = app.cursor.is_captured ? GetMouseWheelMove() : 0.0f;
    app.cursor.suppress_delta = 0;

    const sdk::Orbit_Config& orbit_cfg = app.view_mode == VIEW_LOCAL ? app.config.local.orbit : app.config.birds_eye.orbit;
    sdk::orbit_camera_update(app.orbit, orbit_cfg, mouse_delta, wheel, app.cursor.is_captured);
    handle_selection(app);

    Axis_Label bird_labels[6] = {};
    BeginDrawing();
    ClearBackground(Color{18, 18, 24, 255});
    if (app.view_mode == VIEW_LOCAL) rlSetClipPlanes((double)app.config.local.clip_near, (double)app.config.local.clip_far); else rlSetClipPlanes((double)app.config.birds_eye.clip_near, (double)app.config.birds_eye.clip_far);
    BeginMode3D(app.orbit.camera);
    if (app.view_mode == VIEW_LOCAL) render_local_view(app); else { render_birds_eye_shell(app); draw_birds_eye_compass_3d(app, bird_labels); }
    EndMode3D();
    if (app.view_mode == VIEW_BIRDS_EYE) draw_birds_eye_compass_labels_2d(app, bird_labels);
    draw_overlay(app);
    EndDrawing();

    alloc::arena_reset(app.frame_arena);
}

void application_shutdown(Application& app)
{
    if (app.cursor.is_captured) EnableCursor();
    if (app.owns_font) UnloadFont(app.font);
    cube_data_release(app.data, app.persistent_allocator);
    alloc::allocator_free(app.persistent_allocator, app.frame_memory);
    app.frame_memory = 0;
    sdk::runtime_shutdown(app.runtime);
}

}

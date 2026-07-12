#pragma once

#include "raylib.h"

#include <stdint.h>

namespace sdk {

enum Axis { AXIS_NONE = 0, AXIS_X, AXIS_Y, AXIS_Z, AXIS_COUNT };

enum Face_Direction {
    FACE_DIRECTION_NONE = 0,
    FACE_DIRECTION_POSITIVE_X,
    FACE_DIRECTION_NEGATIVE_X,
    FACE_DIRECTION_POSITIVE_Y,
    FACE_DIRECTION_NEGATIVE_Y,
    FACE_DIRECTION_POSITIVE_Z,
    FACE_DIRECTION_NEGATIVE_Z,
    FACE_DIRECTION_COUNT
};

enum Face_Edge_Mask {
    FACE_EDGE_TOP = 1u << 0,
    FACE_EDGE_RIGHT = 1u << 1,
    FACE_EDGE_BOTTOM = 1u << 2,
    FACE_EDGE_LEFT = 1u << 3,
    FACE_EDGE_ALL = (1u << 4) - 1u
};

// One canonical table supplies face normals, face-local text axes, compass
// labels, and edge orientation. Direction zero is an all-zero safe stub.
struct Axis_Direction_Info {
    Vector3 normal;
    Vector3 right;
    Vector3 down;
    char label[3];
    uint8_t axis;
};

struct Render_Resource_Config {
    const char* font_path;
    int font_pixel_size;
};

// Raylib owns the loaded texture storage. The ownership byte makes the
// all-zero state inert and supports shutdown after partial initialization.
struct Render_Resources {
    Font face_font;
    uint8_t owns_face_font;
    uint8_t reserved[3];
};

struct Cube_Draw {
    Vector3 center;
    float edge_length;
    Color fill_color;
    Color edge_color;
};

// Face commands are transient values. The application computes ordering and
// edge ownership; this SDK function consumes the array immediately.
struct Cube_Face_Draw {
    Vector3 center;
    float edge_length;
    float sort_depth;
    Color fill_color;
    Color edge_color;
    uint8_t direction;
    uint8_t edge_mask;
    uint8_t reserved[2];
};

struct Plane_Text_Draw {
    const char* text;
    Vector3 center;
    Vector3 right;
    Vector3 down;
    float font_size;
    float glyph_spacing;
    float measured_width;
    Color color;
};

struct Plane_Text_Block_Draw {
    const char* const* lines;
    const float* measured_widths;
    uint32_t line_count;
    Vector3 center;
    Vector3 right;
    Vector3 down;
    float font_size;
    float glyph_spacing;
    float line_spacing;
    Color color;
};

struct Billboard_Text_Draw {
    const char* text;
    Vector3 center;
    float font_size;
    float glyph_spacing;
    Color color;
};

struct Arrow_Draw {
    Vector3 origin;
    Vector3 direction;
    float length;
    float shaft_radius;
    float arrowhead_length;
    float arrowhead_radius;
    float gap_center;
    float gap_half_length;
    Color color;
};

const Axis_Direction_Info& axis_direction_info(uint32_t direction);

int render_resources_init(Render_Resources& resources, const Render_Resource_Config& config);
void render_resources_shutdown(Render_Resources& resources);

// Draws an opaque cube and its palette-defined edges. Translucent cubes should
// be expanded to face commands so all faces can be ordered far-to-near.
void draw_cube_immediate(const Cube_Draw& draw);

// Draws opaque faces first, then translucent faces with depth testing retained
// and depth writes disabled, then uniquely owned edges. The caller must supply
// the translucent subset in far-to-near order.
void draw_cube_faces_immediate(const Cube_Face_Draw* draws, uint32_t count);

float measure_plane_text_width(const Render_Resources& resources, const char* text, float font_size,
                               float glyph_spacing);
void draw_plane_text_immediate(const Render_Resources& resources, const Plane_Text_Draw& draw);
void draw_plane_text_block_immediate(const Render_Resources& resources,
                                     const Plane_Text_Block_Draw& draw);
void draw_billboard_text_immediate(const Render_Resources& resources, const Camera3D& camera,
                                   const Billboard_Text_Draw& draw);
void draw_arrow_immediate(const Arrow_Draw& draw);

} // namespace sdk

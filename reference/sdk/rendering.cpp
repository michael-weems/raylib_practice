#include "sdk/rendering.h"

#include "raymath.h"
#include "rlgl.h"

#include <string.h>

namespace sdk {

// cross(down, right) is the outward face normal for every real direction.
static const Axis_Direction_Info AXIS_DIRECTION_INFO[FACE_DIRECTION_COUNT] = {
    {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {'\0', '\0', '\0'}, AXIS_NONE},
    {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {'+', 'x', '\0'}, AXIS_X},
    {{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {'-', 'x', '\0'}, AXIS_X},
    {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {'+', 'y', '\0'}, AXIS_Y},
    {{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {'-', 'y', '\0'}, AXIS_Y},
    {{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {'+', 'z', '\0'}, AXIS_Z},
    {{0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {'-', 'z', '\0'}, AXIS_Z}};

const Axis_Direction_Info& axis_direction_info(uint32_t direction) {
    if (direction >= FACE_DIRECTION_COUNT) {
        direction = FACE_DIRECTION_NONE;
    }

    return AXIS_DIRECTION_INFO[direction];
}

static float clamp_float(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }

    return value;
}

static void cube_face_vertices(Vector3 center, float edge_length, uint32_t direction,
                               Vector3& bottom_left, Vector3& bottom_right, Vector3& top_right,
                               Vector3& top_left) {
    const Axis_Direction_Info& direction_info = axis_direction_info(direction);
    Vector3 face_up = Vector3Negate(direction_info.down);
    float half_edge_length = 0.5f * edge_length;
    Vector3 face_center = Vector3Add(center, Vector3Scale(direction_info.normal, half_edge_length));

    bottom_left =
        Vector3Subtract(face_center, Vector3Scale(direction_info.right, half_edge_length));
    bottom_left = Vector3Subtract(bottom_left, Vector3Scale(face_up, half_edge_length));
    bottom_right = Vector3Add(bottom_left, Vector3Scale(direction_info.right, edge_length));
    top_right = Vector3Add(bottom_right, Vector3Scale(face_up, edge_length));
    top_left = Vector3Add(bottom_left, Vector3Scale(face_up, edge_length));
}

static void emit_line(Vector3 start, Vector3 end, Color color) {
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(start.x, start.y, start.z);
    rlVertex3f(end.x, end.y, end.z);
}

static void emit_opaque_face(const Cube_Face_Draw& draw) {
    Vector3 bottom_left;
    Vector3 bottom_right;
    Vector3 top_right;
    Vector3 top_left;
    const Vector3 normal = axis_direction_info(draw.direction).normal;

    cube_face_vertices(draw.center, draw.edge_length, draw.direction, bottom_left, bottom_right,
                       top_right, top_left);
    rlColor4ub(draw.fill_color.r, draw.fill_color.g, draw.fill_color.b, draw.fill_color.a);
    rlNormal3f(normal.x, normal.y, normal.z);
    rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
    rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
    rlVertex3f(top_right.x, top_right.y, top_right.z);
    rlVertex3f(top_left.x, top_left.y, top_left.z);
}

static void emit_transparent_face(const Cube_Face_Draw& draw) {
    Vector3 bottom_left;
    Vector3 bottom_right;
    Vector3 top_right;
    Vector3 top_left;
    const Vector3 normal = axis_direction_info(draw.direction).normal;

    cube_face_vertices(draw.center, draw.edge_length, draw.direction, bottom_left, bottom_right,
                       top_right, top_left);

    // RLSW may clear primitive alpha state between its internal triangles.
    // Repeat color and normal for both triangles so an entire face blends.
    rlColor4ub(draw.fill_color.r, draw.fill_color.g, draw.fill_color.b, draw.fill_color.a);
    rlNormal3f(normal.x, normal.y, normal.z);
    rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
    rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
    rlVertex3f(top_right.x, top_right.y, top_right.z);

    rlColor4ub(draw.fill_color.r, draw.fill_color.g, draw.fill_color.b, draw.fill_color.a);
    rlNormal3f(normal.x, normal.y, normal.z);
    rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
    rlVertex3f(top_right.x, top_right.y, top_right.z);
    rlVertex3f(top_left.x, top_left.y, top_left.z);
}

int render_resources_init(Render_Resources& resources, const Render_Resource_Config& config) {
    memset(&resources, 0, sizeof(resources));
    if (config.font_path == 0 || config.font_pixel_size <= 0) {
        return 1;
    }

    resources.face_font = LoadFontEx(config.font_path, config.font_pixel_size, 0, 0);
    if (IsFontValid(resources.face_font)) {
        resources.owns_face_font = 1;
        return 0;
    }

    resources.face_font = GetFontDefault();
    if (!IsFontValid(resources.face_font)) {
        memset(&resources, 0, sizeof(resources));
        return 2;
    }

    return 0;
}

void render_resources_shutdown(Render_Resources& resources) {
    if (resources.owns_face_font && IsFontValid(resources.face_font)) {
        UnloadFont(resources.face_font);
    }

    memset(&resources, 0, sizeof(resources));
}

void draw_cube_immediate(const Cube_Draw& draw) {
    if (draw.edge_length <= 0.0f) {
        return;
    }

    Vector3 cube_size = {draw.edge_length, draw.edge_length, draw.edge_length};
    if (draw.fill_color.a == 255u) {
        DrawCubeV(draw.center, cube_size, draw.fill_color);
    }
    if (draw.edge_color.a != 0u) {
        DrawCubeWiresV(draw.center, cube_size, draw.edge_color);
    }
}

void draw_cube_faces_immediate(const Cube_Face_Draw* draws, uint32_t count) {
    if (draws == 0 || count == 0u) {
        return;
    }

    rlSetTexture(0);
    rlBegin(RL_QUADS);
    for (uint32_t index = 0u; index < count; ++index) {
        const Cube_Face_Draw& draw = draws[index];
        if (draw.edge_length <= 0.0f || draw.direction == FACE_DIRECTION_NONE ||
            draw.direction >= FACE_DIRECTION_COUNT) {
            continue;
        }
        if (draw.fill_color.a == 255u) {
            emit_opaque_face(draw);
        }
    }
    rlEnd();

    BeginBlendMode(BLEND_ALPHA);
    rlDisableDepthMask();
    rlBegin(RL_TRIANGLES);
    for (uint32_t index = 0u; index < count; ++index) {
        const Cube_Face_Draw& draw = draws[index];
        if (draw.edge_length <= 0.0f || draw.direction == FACE_DIRECTION_NONE ||
            draw.direction >= FACE_DIRECTION_COUNT) {
            continue;
        }
        if (draw.fill_color.a > 0u && draw.fill_color.a < 255u) {
            emit_transparent_face(draw);
        }
    }
    rlEnd();
    rlEnableDepthMask();
    EndBlendMode();

    rlBegin(RL_LINES);
    for (uint32_t index = 0u; index < count; ++index) {
        const Cube_Face_Draw& draw = draws[index];
        if (draw.edge_length <= 0.0f || draw.direction == FACE_DIRECTION_NONE ||
            draw.direction >= FACE_DIRECTION_COUNT) {
            continue;
        }
        if (draw.edge_mask == 0u || draw.edge_color.a == 0u) {
            continue;
        }

        Vector3 bottom_left;
        Vector3 bottom_right;
        Vector3 top_right;
        Vector3 top_left;
        cube_face_vertices(draw.center, draw.edge_length, draw.direction, bottom_left, bottom_right,
                           top_right, top_left);

        if (draw.edge_mask & FACE_EDGE_TOP) {
            emit_line(top_left, top_right, draw.edge_color);
        }
        if (draw.edge_mask & FACE_EDGE_RIGHT) {
            emit_line(bottom_right, top_right, draw.edge_color);
        }
        if (draw.edge_mask & FACE_EDGE_BOTTOM) {
            emit_line(bottom_left, bottom_right, draw.edge_color);
        }
        if (draw.edge_mask & FACE_EDGE_LEFT) {
            emit_line(bottom_left, top_left, draw.edge_color);
        }
    }
    rlEnd();
}

static float glyph_advance(const Font& font, int glyph_index) {
    int advance = font.glyphs[glyph_index].advanceX;
    if (advance == 0) {
        advance = (int)font.recs[glyph_index].width;
    }

    return (float)advance;
}

float measure_plane_text_width(const Render_Resources& resources, const char* text, float font_size,
                               float glyph_spacing) {
    const Font& font = resources.face_font;
    if (text == 0 || font_size <= 0.0f || !IsFontValid(font)) {
        return 0.0f;
    }

    float scale = font_size / (float)font.baseSize;
    float width = 0.0f;
    const unsigned char* cursor = (const unsigned char*)text;
    while (*cursor != 0u) {
        int glyph_index = GetGlyphIndex(font, *cursor);
        width += glyph_advance(font, glyph_index) * scale;
        if (cursor[1] != 0u) {
            width += glyph_spacing;
        }
        ++cursor;
    }

    return width;
}

static void emit_plane_text_line(const Font& font, const char* text, Vector3 center, Vector3 right,
                                 Vector3 down, float font_size, float glyph_spacing,
                                 float measured_width, Color color) {
    if (text == 0) {
        return;
    }

    float scale = font_size / (float)font.baseSize;
    Vector3 origin = Vector3Subtract(center, Vector3Scale(right, 0.5f * measured_width));
    origin = Vector3Subtract(origin, Vector3Scale(down, 0.5f * font_size));
    Vector3 normal = Vector3CrossProduct(down, right);
    float pen_x = 0.0f;
    const unsigned char* cursor = (const unsigned char*)text;
    while (*cursor != 0u) {
        int glyph_index = GetGlyphIndex(font, *cursor);
        const GlyphInfo& glyph = font.glyphs[glyph_index];
        const Rectangle& glyph_rectangle = font.recs[glyph_index];

        if (*cursor != ' ') {
            float glyph_x = pen_x + (float)(glyph.offsetX - font.glyphPadding) * scale;
            float glyph_y = (float)(glyph.offsetY - font.glyphPadding) * scale;
            float glyph_width = (glyph_rectangle.width + 2.0f * (float)font.glyphPadding) * scale;
            float glyph_height = (glyph_rectangle.height + 2.0f * (float)font.glyphPadding) * scale;
            Vector3 top_left = Vector3Add(origin, Vector3Scale(right, glyph_x));
            top_left = Vector3Add(top_left, Vector3Scale(down, glyph_y));
            Vector3 bottom_left = Vector3Add(top_left, Vector3Scale(down, glyph_height));
            Vector3 bottom_right = Vector3Add(bottom_left, Vector3Scale(right, glyph_width));
            Vector3 top_right = Vector3Add(top_left, Vector3Scale(right, glyph_width));
            float texture_x0 =
                (glyph_rectangle.x - (float)font.glyphPadding) / (float)font.texture.width;
            float texture_y0 =
                (glyph_rectangle.y - (float)font.glyphPadding) / (float)font.texture.height;
            float texture_x1 =
                (glyph_rectangle.x + glyph_rectangle.width + (float)font.glyphPadding) /
                (float)font.texture.width;
            float texture_y1 =
                (glyph_rectangle.y + glyph_rectangle.height + (float)font.glyphPadding) /
                (float)font.texture.height;

            rlColor4ub(color.r, color.g, color.b, color.a);
            rlNormal3f(normal.x, normal.y, normal.z);
            rlTexCoord2f(texture_x0, texture_y0);
            rlVertex3f(top_left.x, top_left.y, top_left.z);
            rlTexCoord2f(texture_x0, texture_y1);
            rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
            rlTexCoord2f(texture_x1, texture_y1);
            rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
            rlTexCoord2f(texture_x1, texture_y0);
            rlVertex3f(top_right.x, top_right.y, top_right.z);
        }

        pen_x += glyph_advance(font, glyph_index) * scale + glyph_spacing;
        ++cursor;
    }
}

void draw_plane_text_block_immediate(const Render_Resources& resources,
                                     const Plane_Text_Block_Draw& draw) {
    const Font& font = resources.face_font;
    if (draw.lines == 0 || draw.line_count == 0u || draw.font_size <= 0.0f || !IsFontValid(font)) {
        return;
    }

    float total_height = (float)draw.line_count * draw.font_size;
    if (draw.line_count > 1u) {
        total_height += (float)(draw.line_count - 1u) * draw.line_spacing;
    }

    rlSetTexture(font.texture.id);
    rlBegin(RL_QUADS);
    for (uint32_t line_index = 0u; line_index < draw.line_count; ++line_index) {
        const char* text = draw.lines[line_index];
        if (text == 0 || text[0] == '\0') {
            continue;
        }

        float measured_width = 0.0f;
        if (draw.measured_widths != 0) {
            measured_width = draw.measured_widths[line_index];
        }
        if (measured_width <= 0.0f) {
            measured_width =
                measure_plane_text_width(resources, text, draw.font_size, draw.glyph_spacing);
        }

        float line_offset = -0.5f * total_height + 0.5f * draw.font_size;
        line_offset += (float)line_index * (draw.font_size + draw.line_spacing);
        Vector3 line_center = Vector3Add(draw.center, Vector3Scale(draw.down, line_offset));
        emit_plane_text_line(font, text, line_center, draw.right, draw.down, draw.font_size,
                             draw.glyph_spacing, measured_width, draw.color);
    }
    rlEnd();
    rlSetTexture(0);
}

void draw_plane_text_immediate(const Render_Resources& resources, const Plane_Text_Draw& draw) {
    const char* lines[1] = {draw.text};
    float measured_widths[1] = {draw.measured_width};
    Plane_Text_Block_Draw block_draw = {};
    block_draw.lines = lines;
    block_draw.measured_widths = measured_widths;
    block_draw.line_count = 1u;
    block_draw.center = draw.center;
    block_draw.right = draw.right;
    block_draw.down = draw.down;
    block_draw.font_size = draw.font_size;
    block_draw.glyph_spacing = draw.glyph_spacing;
    block_draw.color = draw.color;
    draw_plane_text_block_immediate(resources, block_draw);
}

void draw_billboard_text_immediate(const Render_Resources& resources, const Camera3D& camera,
                                   const Billboard_Text_Draw& draw) {
    Vector3 camera_forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camera_right = Vector3Normalize(Vector3CrossProduct(camera_forward, camera.up));
    Vector3 camera_view_up = Vector3Normalize(Vector3CrossProduct(camera_right, camera_forward));

    Plane_Text_Draw plane_draw = {};
    plane_draw.text = draw.text;
    plane_draw.center = draw.center;
    plane_draw.right = camera_right;
    plane_draw.down = Vector3Negate(camera_view_up);
    plane_draw.font_size = draw.font_size;
    plane_draw.glyph_spacing = draw.glyph_spacing;
    plane_draw.color = draw.color;
    draw_plane_text_immediate(resources, plane_draw);
}

void draw_arrow_immediate(const Arrow_Draw& draw) {
    if (draw.length <= 0.0f || draw.arrowhead_length < 0.0f ||
        draw.arrowhead_length > draw.length) {
        return;
    }
    if (Vector3LengthSqr(draw.direction) < 0.000001f) {
        return;
    }

    Vector3 direction = Vector3Normalize(draw.direction);
    float shaft_length = draw.length - draw.arrowhead_length;
    float gap_half_length = clamp_float(draw.gap_half_length, 0.0f, 0.5f * shaft_length);
    float gap_start_distance = clamp_float(draw.gap_center - gap_half_length, 0.0f, shaft_length);
    float gap_end_distance = clamp_float(draw.gap_center + gap_half_length, 0.0f, shaft_length);
    Vector3 shaft_end = Vector3Add(draw.origin, Vector3Scale(direction, shaft_length));
    Vector3 arrow_tip = Vector3Add(draw.origin, Vector3Scale(direction, draw.length));

    if (gap_start_distance > 0.0f) {
        Vector3 gap_start = Vector3Add(draw.origin, Vector3Scale(direction, gap_start_distance));
        DrawCylinderEx(draw.origin, gap_start, draw.shaft_radius, draw.shaft_radius, 8, draw.color);
    }
    if (gap_end_distance < shaft_length) {
        Vector3 gap_end = Vector3Add(draw.origin, Vector3Scale(direction, gap_end_distance));
        DrawCylinderEx(gap_end, shaft_end, draw.shaft_radius, draw.shaft_radius, 8, draw.color);
    }

    DrawCylinderEx(shaft_end, arrow_tip, draw.arrowhead_radius, 0.0f, 8, draw.color);
}

} // namespace sdk

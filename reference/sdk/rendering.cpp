#include "sdk/rendering.h"

#include "raymath.h"
#include "rlgl.h"

#include <math.h>
#include <stddef.h>

namespace sdk {

static float distance_squared(Vector3 a, Vector3 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return dx*dx + dy*dy + dz*dz;
}

static void copy_label_text(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0) {
        return;
    }

    dst[0] = '\0';

    if (!src) {
        return;
    }

    size_t i = 0;
    for (; i + 1 < dst_size && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

void billboard_label_push(
    Billboard_Label_Buffer& buffer,
    Vector3 world_position,
    const char* text,
    float font_size,
    Color color)
{
    billboard_label_push_ex(buffer, world_position, text, font_size, color, BLANK, 0.0f);
}

void billboard_label_push_ex(
    Billboard_Label_Buffer& buffer,
    Vector3 world_position,
    const char* text,
    float font_size,
    Color color,
    Color background_color,
    float background_padding)
{
    if (!buffer.labels || buffer.count >= buffer.capacity) {
        return;
    }

    Billboard_Label* label = buffer.labels + buffer.count;
    label->world_position = world_position;
    label->color = color;
    label->background_color = background_color;
    label->font_size = font_size;
    label->background_padding = background_padding;
    copy_label_text(label->text, sizeof(label->text), text);

    ++buffer.count;
}

static float measure_text_width_3d(Font font, const char* text, float font_size, float spacing)
{
    if (!text) {
        return 0.0f;
    }

    float scale = font_size/(float)font.baseSize;
    float width = 0.0f;
    int length = TextLength(text);

    for (int i = 0; i < length;) {
        int codepoint_byte_count = 0;
        int codepoint = GetCodepoint(&text[i], &codepoint_byte_count);
        if (codepoint == 0x3f) {
            codepoint_byte_count = 1;
        }

        if (codepoint == '\n') {
            break;
        }

        int index = GetGlyphIndex(font, codepoint);
        if (font.glyphs[index].advanceX == 0) {
            width += (float)font.recs[index].width * scale + spacing;
        } else {
            width += (float)font.glyphs[index].advanceX * scale + spacing;
        }

        i += codepoint_byte_count;
    }

    if (width > 0.0f) {
        width -= spacing;
    }

    return width;
}

static Vector3 plane_point(Vector3 origin, Vector3 right, float x, Vector3 up, float y)
{
    return Vector3Add(Vector3Add(origin, Vector3Scale(right, x)), Vector3Scale(up, y));
}

static void draw_filled_plane_3d(
    Vector3 center,
    Vector3 right,
    Vector3 up,
    float width,
    float height,
    Color color)
{
    Vector3 half_right = Vector3Scale(Vector3Normalize(right), width * 0.5f);
    Vector3 half_up = Vector3Scale(Vector3Normalize(up), height * 0.5f);

    Vector3 top_left = Vector3Add(Vector3Subtract(center, half_right), half_up);
    Vector3 bottom_left = Vector3Subtract(Vector3Subtract(center, half_right), half_up);
    Vector3 bottom_right = Vector3Subtract(Vector3Add(center, half_right), half_up);
    Vector3 top_right = Vector3Add(Vector3Add(center, half_right), half_up);

    BeginBlendMode(BLEND_ALPHA);
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    rlSetTexture(0);

    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex3f(top_left.x, top_left.y, top_left.z);
        rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
        rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
        rlVertex3f(top_right.x, top_right.y, top_right.z);
    rlEnd();

    rlEnableDepthMask();
    rlEnableBackfaceCulling();
    EndBlendMode();
}

static void draw_text_codepoint_on_plane_3d(
    Font font,
    int codepoint,
    Vector3 origin,
    Vector3 right,
    Vector3 up,
    float font_size,
    Color tint)
{
    int index = GetGlyphIndex(font, codepoint);
    float scale = font_size/(float)font.baseSize;

    Rectangle src = {};
    src.x = font.recs[index].x - (float)font.glyphPadding;
    src.y = font.recs[index].y - (float)font.glyphPadding;
    src.width = font.recs[index].width + 2.0f*(float)font.glyphPadding;
    src.height = font.recs[index].height + 2.0f*(float)font.glyphPadding;

    float offset_x = (float)(font.glyphs[index].offsetX - font.glyphPadding) * scale;
    float offset_y = (float)(font.glyphs[index].offsetY - font.glyphPadding) * scale;
    float width = src.width * scale;
    float height = src.height * scale;

    Vector3 down = Vector3Scale(up, -1.0f);
    Vector3 top_left = plane_point(origin, right, offset_x, down, offset_y);
    Vector3 bottom_left = Vector3Add(top_left, Vector3Scale(down, height));
    Vector3 bottom_right = Vector3Add(bottom_left, Vector3Scale(right, width));
    Vector3 top_right = Vector3Add(top_left, Vector3Scale(right, width));

    float tx = src.x/(float)font.texture.width;
    float ty = src.y/(float)font.texture.height;
    float tw = (src.x + src.width)/(float)font.texture.width;
    float th = (src.y + src.height)/(float)font.texture.height;

    rlCheckRenderBatchLimit(8);
    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        rlTexCoord2f(tx, ty); rlVertex3f(top_left.x, top_left.y, top_left.z);
        rlTexCoord2f(tx, th); rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
        rlTexCoord2f(tw, th); rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
        rlTexCoord2f(tw, ty); rlVertex3f(top_right.x, top_right.y, top_right.z);

        rlTexCoord2f(tx, ty); rlVertex3f(top_left.x, top_left.y, top_left.z);
        rlTexCoord2f(tw, ty); rlVertex3f(top_right.x, top_right.y, top_right.z);
        rlTexCoord2f(tw, th); rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
        rlTexCoord2f(tx, th); rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
    rlEnd();
}

void draw_text_on_plane_3d(
    const char* text,
    Vector3 center,
    Vector3 right,
    Vector3 up,
    float font_size,
    float spacing,
    Color color)
{
    if (!text || text[0] == '\0') {
        return;
    }

    Font font = GetFontDefault();
    if (font.texture.id == 0) {
        return;
    }

    right = Vector3Normalize(right);
    up = Vector3Normalize(up);

    float width = measure_text_width_3d(font, text, font_size, spacing);
    Vector3 origin = center;
    origin = Vector3Subtract(origin, Vector3Scale(right, width * 0.5f));
    origin = Vector3Add(origin, Vector3Scale(up, font_size * 0.5f));

    float x = 0.0f;
    float scale = font_size/(float)font.baseSize;
    int length = TextLength(text);

    BeginBlendMode(BLEND_ALPHA);
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    rlSetTexture(font.texture.id);

    for (int i = 0; i < length;) {
        int codepoint_byte_count = 0;
        int codepoint = GetCodepoint(&text[i], &codepoint_byte_count);
        if (codepoint == 0x3f) {
            codepoint_byte_count = 1;
        }

        if (codepoint == '\n') {
            break;
        }

        int index = GetGlyphIndex(font, codepoint);
        if (codepoint != ' ' && codepoint != '\t') {
            Vector3 glyph_origin = Vector3Add(origin, Vector3Scale(right, x));
            draw_text_codepoint_on_plane_3d(font, codepoint, glyph_origin, right, up, font_size, color);
        }

        if (font.glyphs[index].advanceX == 0) {
            x += (float)font.recs[index].width * scale + spacing;
        } else {
            x += (float)font.glyphs[index].advanceX * scale + spacing;
        }

        i += codepoint_byte_count;
    }

    rlSetTexture(0);
    rlEnableDepthMask();
    rlEnableBackfaceCulling();
    EndBlendMode();
}

void draw_billboard_labels_3d(const Camera3D& camera, const Billboard_Label_Buffer& buffer)
{
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    if (Vector3LengthSqr(right) < 0.0001f) {
        right = Vector3{ 1.0f, 0.0f, 0.0f };
    }

    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

    for (int i = 0; i < buffer.count; ++i) {
        const Billboard_Label& label = buffer.labels[i];
        float spacing = label.font_size * 0.08f;

        if (label.background_color.a > 0) {
            Font font = GetFontDefault();
            float width = measure_text_width_3d(font, label.text, label.font_size, spacing);
            float background_width = width + label.background_padding * 2.0f;
            float background_height = label.font_size + label.background_padding * 2.0f;
            Vector3 background_position = Vector3Add(label.world_position, Vector3Scale(forward, label.font_size * 0.035f));
            draw_filled_plane_3d(background_position, right, up, background_width, background_height, label.background_color);
        }

        draw_text_on_plane_3d(label.text, label.world_position, right, up, label.font_size, spacing, label.color);
    }
}

static int clamp_int(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

void draw_billboard_labels_screen_overlay(const Camera3D& camera, const Billboard_Label_Buffer& buffer)
{
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    if (Vector3LengthSqr(right) < 0.0001f) {
        right = Vector3{ 1.0f, 0.0f, 0.0f };
    }

    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

    for (int i = 0; i < buffer.count; ++i) {
        const Billboard_Label& label = buffer.labels[i];

        Vector2 center = GetWorldToScreen(label.world_position, camera);
        Vector2 size_sample = GetWorldToScreen(Vector3Add(label.world_position, Vector3Scale(up, label.font_size)), camera);

        float dx = size_sample.x - center.x;
        float dy = size_sample.y - center.y;
        int font_pixels = clamp_int((int)(sqrtf(dx*dx + dy*dy) + 0.5f), 24, 160);
        int text_width = MeasureText(label.text, font_pixels);
        int text_height = font_pixels;
        int padding = (int)(((label.background_padding / label.font_size) * (float)font_pixels) + 0.5f);
        padding = clamp_int(padding, 4, 64);

        int x = (int)(center.x + 0.5f) - text_width/2;
        int y = (int)(center.y + 0.5f) - text_height/2;

        if (label.background_color.a > 0) {
            DrawRectangle(x - padding, y - padding, text_width + padding*2, text_height + padding*2, label.background_color);
        }

        DrawText(label.text, x, y, font_pixels, label.color);
    }
}

struct Cube_Face {
    Vector3 a;
    Vector3 b;
    Vector3 c;
    Vector3 d;
    Vector3 center;
    float distance_to_camera_sq;
};

static void draw_face(const Cube_Face& face, Color color)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex3f(face.a.x, face.a.y, face.a.z);
        rlVertex3f(face.b.x, face.b.y, face.b.z);
        rlVertex3f(face.c.x, face.c.y, face.c.z);

        rlVertex3f(face.a.x, face.a.y, face.a.z);
        rlVertex3f(face.c.x, face.c.y, face.c.z);
        rlVertex3f(face.d.x, face.d.y, face.d.z);
    rlEnd();
}

static void draw_transparent_cube_faces_sorted(const Camera3D& camera, Vector3 center, Vector3 size, Color fill)
{
    float hx = size.x * 0.5f;
    float hy = size.y * 0.5f;
    float hz = size.z * 0.5f;

    Vector3 v[8] = {};
    v[0] = Vector3{ center.x - hx, center.y - hy, center.z - hz };
    v[1] = Vector3{ center.x + hx, center.y - hy, center.z - hz };
    v[2] = Vector3{ center.x + hx, center.y + hy, center.z - hz };
    v[3] = Vector3{ center.x - hx, center.y + hy, center.z - hz };
    v[4] = Vector3{ center.x - hx, center.y - hy, center.z + hz };
    v[5] = Vector3{ center.x + hx, center.y - hy, center.z + hz };
    v[6] = Vector3{ center.x + hx, center.y + hy, center.z + hz };
    v[7] = Vector3{ center.x - hx, center.y + hy, center.z + hz };

    Cube_Face faces[6] = {};
    faces[0] = Cube_Face{ v[1], v[5], v[6], v[2], Vector3{ center.x + hx, center.y, center.z }, 0.0f };
    faces[1] = Cube_Face{ v[4], v[0], v[3], v[7], Vector3{ center.x - hx, center.y, center.z }, 0.0f };
    faces[2] = Cube_Face{ v[3], v[2], v[6], v[7], Vector3{ center.x, center.y + hy, center.z }, 0.0f };
    faces[3] = Cube_Face{ v[4], v[5], v[1], v[0], Vector3{ center.x, center.y - hy, center.z }, 0.0f };
    faces[4] = Cube_Face{ v[5], v[4], v[7], v[6], Vector3{ center.x, center.y, center.z + hz }, 0.0f };
    faces[5] = Cube_Face{ v[0], v[1], v[2], v[3], Vector3{ center.x, center.y, center.z - hz }, 0.0f };

    for (int i = 0; i < 6; ++i) {
        faces[i].distance_to_camera_sq = distance_squared(camera.position, faces[i].center);
    }

    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 6; ++j) {
            if (faces[i].distance_to_camera_sq < faces[j].distance_to_camera_sq) {
                Cube_Face tmp = faces[i];
                faces[i] = faces[j];
                faces[j] = tmp;
            }
        }
    }

    BeginBlendMode(BLEND_ALPHA);
    rlDisableBackfaceCulling();
    rlDisableDepthMask();

    for (int i = 0; i < 6; ++i) {
        draw_face(faces[i], fill);
    }

    rlEnableDepthMask();
    rlEnableBackfaceCulling();
    EndBlendMode();
}

void draw_cube_software_friendly(
    const Camera3D& camera,
    Vector3 center,
    Vector3 size,
    Color fill,
    Color edge)
{
    if (fill.a == 255) {
        DrawCubeV(center, size, fill);
    } else {
        draw_transparent_cube_faces_sorted(camera, center, size, fill);
    }

    DrawCubeWiresV(center, size, edge);
}

static Vector3 axis_vector(int axis, int sign)
{
    Vector3 v = {};
    if (axis == 0) {
        v.x = (float)sign;
    } else if (axis == 1) {
        v.y = (float)sign;
    } else {
        v.z = (float)sign;
    }
    return v;
}

static void draw_axis_aligned_box_segment(
    Vector3 origin,
    int axis,
    int sign,
    float a,
    float b,
    float thickness,
    Color color)
{
    Vector3 dir = axis_vector(axis, sign);
    Vector3 center = Vector3Add(origin, Vector3Scale(dir, (a + b) * 0.5f));

    Vector3 size = Vector3{ thickness, thickness, thickness };
    if (axis == 0) {
        size.x = b - a;
    } else if (axis == 1) {
        size.y = b - a;
    } else {
        size.z = b - a;
    }

    DrawCubeV(center, size, color);
}

void draw_axis_compass_arrow(
    Billboard_Label_Buffer& labels,
    Vector3 origin,
    int axis,
    int sign,
    float length,
    float thickness,
    float label_gap,
    Color axis_color,
    const char* label)
{
    draw_axis_compass_arrow_ex(
        labels,
        origin,
        axis,
        sign,
        length,
        thickness,
        label_gap,
        axis_color,
        label,
        thickness * 4.0f,
        WHITE,
        BLANK,
        0.0f);
}

void draw_axis_compass_arrow_ex(
    Billboard_Label_Buffer& labels,
    Vector3 origin,
    int axis,
    int sign,
    float length,
    float thickness,
    float label_gap,
    Color axis_color,
    const char* label,
    float label_font_size,
    Color label_color,
    Color label_background_color,
    float label_background_padding)
{
    Vector3 dir = axis_vector(axis, sign);
    float head_length = thickness * 5.0f;
    float middle = length * 0.5f;
    float gap_min = middle - label_gap * 0.5f;
    float gap_max = middle + label_gap * 0.5f;
    float shaft_end = length - head_length;

    if (gap_min > 0.0f) {
        draw_axis_aligned_box_segment(origin, axis, sign, 0.0f, gap_min, thickness, axis_color);
    }

    if (gap_max < shaft_end) {
        draw_axis_aligned_box_segment(origin, axis, sign, gap_max, shaft_end, thickness, axis_color);
    }

    Vector3 head_start = Vector3Add(origin, Vector3Scale(dir, shaft_end));
    Vector3 head_end = Vector3Add(origin, Vector3Scale(dir, length));
    DrawCylinderEx(head_start, head_end, thickness * 2.5f, 0.0f, 12, axis_color);

    if (label && label[0] != '\0') {
        billboard_label_push_ex(
            labels,
            Vector3Add(origin, Vector3Scale(dir, middle)),
            label,
            label_font_size,
            label_color,
            label_background_color,
            label_background_padding);
    }
}

void draw_boundary_grid_planes(Vector3 field_min, Vector3 field_max, float spacing, Color color)
{
    float coarse = spacing * 25.0f;
    if (coarse <= 0.0f) {
        coarse = 1.0f;
    }

    for (float x = field_min.x; x <= field_max.x + 0.001f; x += coarse) {
        DrawLine3D(Vector3{ x, field_min.y, field_min.z }, Vector3{ x, field_min.y, field_max.z }, color);
        DrawLine3D(Vector3{ x, field_max.y, field_min.z }, Vector3{ x, field_max.y, field_max.z }, color);
        DrawLine3D(Vector3{ x, field_min.y, field_min.z }, Vector3{ x, field_max.y, field_min.z }, color);
        DrawLine3D(Vector3{ x, field_min.y, field_max.z }, Vector3{ x, field_max.y, field_max.z }, color);
    }

    for (float y = field_min.y; y <= field_max.y + 0.001f; y += coarse) {
        DrawLine3D(Vector3{ field_min.x, y, field_min.z }, Vector3{ field_min.x, y, field_max.z }, color);
        DrawLine3D(Vector3{ field_max.x, y, field_min.z }, Vector3{ field_max.x, y, field_max.z }, color);
        DrawLine3D(Vector3{ field_min.x, y, field_min.z }, Vector3{ field_max.x, y, field_min.z }, color);
        DrawLine3D(Vector3{ field_min.x, y, field_max.z }, Vector3{ field_max.x, y, field_max.z }, color);
    }

    for (float z = field_min.z; z <= field_max.z + 0.001f; z += coarse) {
        DrawLine3D(Vector3{ field_min.x, field_min.y, z }, Vector3{ field_max.x, field_min.y, z }, color);
        DrawLine3D(Vector3{ field_min.x, field_max.y, z }, Vector3{ field_max.x, field_max.y, z }, color);
        DrawLine3D(Vector3{ field_min.x, field_min.y, z }, Vector3{ field_min.x, field_max.y, z }, color);
        DrawLine3D(Vector3{ field_max.x, field_min.y, z }, Vector3{ field_max.x, field_max.y, z }, color);
    }
}

} // namespace sdk

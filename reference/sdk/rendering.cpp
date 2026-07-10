#include "sdk/rendering.h"

#include "rlgl.h"

#include <math.h>
#include <string.h>

namespace sdk {

static Vector3 v3_add(Vector3 a, Vector3 b) { return Vector3{a.x+b.x, a.y+b.y, a.z+b.z}; }
static Vector3 v3_sub(Vector3 a, Vector3 b) { return Vector3{a.x-b.x, a.y-b.y, a.z-b.z}; }
static Vector3 v3_mul(Vector3 a, float s) { return Vector3{a.x*s, a.y*s, a.z*s}; }
static float v3_dot(Vector3 a, Vector3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static Vector3 v3_cross(Vector3 a, Vector3 b) { return Vector3{a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
static float v3_len(Vector3 a) { return sqrtf(v3_dot(a, a)); }
static Vector3 v3_norm(Vector3 a)
{
    float length = v3_len(a);
    if (length > 0.0f) {
        return v3_mul(a, 1.0f/length);
    }

    return Vector3{0.0f, 0.0f, 0.0f};
}

static void rl_vertex(Vector3 p)
{
    rlVertex3f(p.x, p.y, p.z);
}

static void emit_quad(Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color color)
{
    rlColor4ub(color.r, color.g, color.b, color.a);
    rl_vertex(a);
    rl_vertex(b);
    rl_vertex(c);
    rl_vertex(d);
}

static Vector3 text_point(Vector3 origin, Vector3 right, Vector3 up, float x, float y)
{
    return v3_add(v3_add(origin, v3_mul(right, x)), v3_mul(up, -y));
}

static float text_width_world(Font font, const char* text, float scale, float spacing_world)
{
    float x = 0.0f;
    if (!text) {
        return 0.0f;
    }

    for (const char* at = text; *at; ++at) {
        int gi = GetGlyphIndex(font, (unsigned char)*at);
        Rectangle rec = font.recs[gi];
        GlyphInfo g = font.glyphs[gi];
        int advance = g.advanceX;
        if (!advance) {
            advance = (int)rec.width;
        }

        x += (float)advance*scale + spacing_world;
    }
    if (x > 0.0f) {
        return x - spacing_world;
    }

    return 0.0f;
}

void draw_box_faces(Vector3 mn, Vector3 mx, unsigned int face_mask, Color color)
{
    Vector3 p000 = {mn.x, mn.y, mn.z};
    Vector3 p001 = {mn.x, mn.y, mx.z};
    Vector3 p010 = {mn.x, mx.y, mn.z};
    Vector3 p011 = {mn.x, mx.y, mx.z};
    Vector3 p100 = {mx.x, mn.y, mn.z};
    Vector3 p101 = {mx.x, mn.y, mx.z};
    Vector3 p110 = {mx.x, mx.y, mn.z};
    Vector3 p111 = {mx.x, mx.y, mx.z};

    rlBegin(RL_QUADS);
    if (face_mask & FACE_NEG_X) {
        emit_quad(p000, p001, p011, p010, color);
    }
    if (face_mask & FACE_POS_X) {
        emit_quad(p100, p110, p111, p101, color);
    }
    if (face_mask & FACE_NEG_Y) {
        emit_quad(p000, p100, p101, p001, color);
    }
    if (face_mask & FACE_POS_Y) {
        emit_quad(p010, p011, p111, p110, color);
    }
    if (face_mask & FACE_NEG_Z) {
        emit_quad(p000, p010, p110, p100, color);
    }
    if (face_mask & FACE_POS_Z) {
        emit_quad(p001, p101, p111, p011, color);
    }
    rlEnd();
}

void draw_box_edges(Vector3 mn, Vector3 mx, Color c)
{
    Vector3 p000 = {mn.x, mn.y, mn.z};
    Vector3 p001 = {mn.x, mn.y, mx.z};
    Vector3 p010 = {mn.x, mx.y, mn.z};
    Vector3 p011 = {mn.x, mx.y, mx.z};
    Vector3 p100 = {mx.x, mn.y, mn.z};
    Vector3 p101 = {mx.x, mn.y, mx.z};
    Vector3 p110 = {mx.x, mx.y, mn.z};
    Vector3 p111 = {mx.x, mx.y, mx.z};

    DrawLine3D(p000, p001, c);
    DrawLine3D(p001, p011, c);
    DrawLine3D(p011, p010, c);
    DrawLine3D(p010, p000, c);
    DrawLine3D(p100, p101, c);
    DrawLine3D(p101, p111, c);
    DrawLine3D(p111, p110, c);
    DrawLine3D(p110, p100, c);
    DrawLine3D(p000, p100, c);
    DrawLine3D(p001, p101, c);
    DrawLine3D(p010, p110, c);
    DrawLine3D(p011, p111, c);
}

void draw_text_lines_3d(Font font, const char** lines, int line_count, Vector3 center, Vector3 right, Vector3 up, const Text3D_Style& s, float line_step_world)
{
    if (!lines || line_count <= 0 || font.texture.id == 0 || font.baseSize == 0) {
        return;
    }

    float scale = s.font_size_world/(float)font.baseSize;
    float max_width = 0.0f;
    if (line_step_world <= 0.0f) {
        line_step_world = s.font_size_world*1.25f;
    }

    for (int i = 0; i < line_count; ++i) {
        float w = text_width_world(font, lines[i], scale, s.spacing_world);
        if (w > max_width) {
            max_width = w;
        }
    }

    if (s.draw_background) {
        float block_height = s.font_size_world + line_step_world*(float)(line_count - 1);
        Vector3 origin = v3_add(v3_sub(center, v3_mul(right, max_width*0.5f)), v3_mul(up, block_height*0.5f));
        Vector3 a = v3_add(v3_sub(origin, v3_mul(right, s.spacing_world)), v3_mul(up, s.spacing_world));
        Vector3 b = v3_add(a, v3_mul(right, max_width + s.spacing_world*2.0f));
        Vector3 c = v3_sub(b, v3_mul(up, block_height + s.spacing_world*2.0f));
        Vector3 d = v3_sub(a, v3_mul(up, block_height + s.spacing_world*2.0f));
        rlSetTexture(0);
        rlBegin(RL_QUADS);
        emit_quad(a, d, c, b, s.background_color);
        rlEnd();
    }

    rlSetTexture(font.texture.id);
    rlBegin(RL_QUADS);

    for (int line = 0; line < line_count; ++line) {
        const char* text = lines[line];
        if (!text || !text[0]) {
            continue;
        }

        float w = text_width_world(font, text, scale, s.spacing_world);
        float yoff = ((float)(line_count - 1)*0.5f - (float)line)*line_step_world;
        Vector3 line_center = v3_add(center, v3_mul(up, yoff));
        Vector3 origin = v3_add(v3_sub(line_center, v3_mul(right, w*0.5f)), v3_mul(up, s.font_size_world*0.5f));

        float x = 0.0f;
        for (const char* at = text; *at; ++at) {
            int codepoint = (unsigned char)*at;
            int gi = GetGlyphIndex(font, codepoint);
            Rectangle rec = font.recs[gi];
            GlyphInfo g = font.glyphs[gi];
            float gx = x + (float)g.offsetX*scale;
            float gy = (float)g.offsetY*scale;
            float gw = rec.width*scale;
            float gh = rec.height*scale;
            Vector3 p0 = text_point(origin, right, up, gx,    gy);
            Vector3 p1 = text_point(origin, right, up, gx+gw, gy);
            Vector3 p2 = text_point(origin, right, up, gx+gw, gy+gh);
            Vector3 p3 = text_point(origin, right, up, gx,    gy+gh);
            float tx0 = rec.x/(float)font.texture.width, ty0 = rec.y/(float)font.texture.height;
            float tx1 = (rec.x + rec.width)/(float)font.texture.width, ty1 = (rec.y + rec.height)/(float)font.texture.height;
            rlColor4ub(s.text_color.r, s.text_color.g, s.text_color.b, s.text_color.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);
            rlTexCoord2f(tx0, ty0);
            rl_vertex(p0);
            rlTexCoord2f(tx0, ty1);
            rl_vertex(p3);
            rlTexCoord2f(tx1, ty1);
            rl_vertex(p2);
            rlTexCoord2f(tx1, ty0);
            rl_vertex(p1);
            int advance = g.advanceX;
            if (!advance) {
                advance = (int)rec.width;
            }

            x += (float)advance*scale + s.spacing_world;
        }
    }

    rlEnd();
    rlSetTexture(0);
}

void draw_billboard_text_3d(Camera3D camera, Font font, const char* text, Vector3 center, const Text3D_Style& style)
{
    Vector3 forward = v3_norm(v3_sub(camera.position, center));
    Vector3 right = v3_norm(v3_cross(camera.up, forward));
    Vector3 up = v3_norm(v3_cross(forward, right));
    const char* lines[1] = { text };
    draw_text_lines_3d(font, lines, 1, center, right, up, style, style.font_size_world);
}

void draw_arrow_with_gap(Vector3 origin, Vector3 dir, float start_offset, float length, float gap, float radius, Color color)
{
    Vector3 n = v3_norm(dir);
    float tip = length;
    float head = length*0.18f;
    float half_gap = gap*0.5f;
    Vector3 a0 = v3_add(origin, v3_mul(n, start_offset));
    Vector3 a1 = v3_add(origin, v3_mul(n, length*0.5f - half_gap));
    Vector3 b0 = v3_add(origin, v3_mul(n, length*0.5f + half_gap));
    Vector3 b1 = v3_add(origin, v3_mul(n, tip - head));
    Vector3 t0 = b1;
    Vector3 t1 = v3_add(origin, v3_mul(n, tip));
    if (length*0.5f - half_gap - start_offset > 0.01f) {
        DrawCylinderEx(a0, a1, radius, radius, 10, color);
    }
    if (tip - head - (length*0.5f + half_gap) > 0.01f) {
        DrawCylinderEx(b0, b1, radius, radius, 10, color);
    }

    DrawCylinderEx(t0, t1, radius*3.0f, 0.0f, 16, color);
}

}

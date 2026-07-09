#pragma once

#include "raylib.h"

namespace sdk {

enum Face_Mask {
    FACE_NEG_X = 1 << 0,
    FACE_POS_X = 1 << 1,
    FACE_NEG_Y = 1 << 2,
    FACE_POS_Y = 1 << 3,
    FACE_NEG_Z = 1 << 4,
    FACE_POS_Z = 1 << 5,
    FACE_ALL   = FACE_NEG_X | FACE_POS_X | FACE_NEG_Y | FACE_POS_Y | FACE_NEG_Z | FACE_POS_Z
};

struct Text3D_Style {
    float font_size_world;
    float spacing_world;
    Color text_color;
    Color background_color;
    int draw_background;
};

void set_clip_planes(float near_plane, float far_plane);
void draw_box_faces(Vector3 min, Vector3 max, unsigned int face_mask, Color color);
void draw_box_edges(Vector3 min, Vector3 max, Color color);
void draw_text_3d(Font font, const char* text, Vector3 center, Vector3 right, Vector3 up, const Text3D_Style& style);
void draw_text_lines_3d(Font font, const char** lines, int line_count, Vector3 center, Vector3 right, Vector3 up, const Text3D_Style& style, float line_step_world);
void draw_billboard_text_3d(Camera3D camera, Font font, const char* text, Vector3 center, const Text3D_Style& style);
void draw_arrow_with_gap(Vector3 origin, Vector3 dir, float start_offset, float length, float gap, float radius, Color color);

}

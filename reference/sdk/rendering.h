#ifndef REFERENCE_SDK_RENDERING_H
#define REFERENCE_SDK_RENDERING_H

#include "raylib.h"

namespace sdk {

struct Billboard_Label {
    Vector3 world_position;
    Color color;
    Color background_color;
    float font_size;
    float background_padding;
    char text[64];
};

struct Billboard_Label_Buffer {
    Billboard_Label* labels;
    int capacity;
    int count;
};

void billboard_label_push(
    Billboard_Label_Buffer& buffer,
    Vector3 world_position,
    const char* text,
    float font_size,
    Color color);

void billboard_label_push_ex(
    Billboard_Label_Buffer& buffer,
    Vector3 world_position,
    const char* text,
    float font_size,
    Color color,
    Color background_color,
    float background_padding);

void draw_billboard_labels_3d(const Camera3D& camera, const Billboard_Label_Buffer& buffer);
void draw_billboard_labels_screen_overlay(const Camera3D& camera, const Billboard_Label_Buffer& buffer);

void draw_text_on_plane_3d(
    const char* text,
    Vector3 center,
    Vector3 right,
    Vector3 up,
    float font_size,
    float spacing,
    Color color);

void draw_cube_software_friendly(
    const Camera3D& camera,
    Vector3 center,
    Vector3 size,
    Color fill,
    Color edge);

void draw_axis_compass_arrow(
    Billboard_Label_Buffer& labels,
    Vector3 origin,
    int axis,
    int sign,
    float length,
    float thickness,
    float label_gap,
    Color axis_color,
    const char* label);

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
    float label_background_padding);

void draw_boundary_grid_planes(
    Vector3 field_min,
    Vector3 field_max,
    float spacing,
    Color color);

} // namespace sdk

#endif // REFERENCE_SDK_RENDERING_H

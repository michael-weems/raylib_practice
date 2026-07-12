#pragma once

#include "raylib.h"

#include <stdint.h>

namespace sdk {

struct Orbit_Config {
    float mouse_sensitivity;
    float wheel_speed;
    float minimum_pitch;
    float maximum_pitch;
    float minimum_distance;
    float maximum_distance;
    float vertical_fov;
};

// Target, yaw, pitch, and distance are authoritative. Camera3D is derived on
// demand so rendering and picking can share one application-owned snapshot.
struct Orbit_Camera {
    Vector3 target;
    float yaw;
    float pitch;
    float distance;
    float vertical_fov;
    int projection;
};

// Logical capture survives focus loss. Physical capture follows focus and is
// changed only on transitions so Raylib's cursor state cannot flicker.
struct Cursor_Capture {
    uint8_t wants_capture;
    uint8_t is_captured;
    uint8_t was_window_focused;
    uint8_t suppress_mouse_delta;
};

void cursor_capture_update(Cursor_Capture& cursor, int wants_capture, int window_focused);
void cursor_capture_shutdown(Cursor_Capture& cursor);

void orbit_camera_init(Orbit_Camera& orbit, Vector3 target, float yaw, float pitch, float distance,
                       float vertical_fov);
void orbit_camera_update(Orbit_Camera& orbit, const Orbit_Config& config, Vector2 mouse_delta,
                         float mouse_wheel, int rotate_from_mouse);
void orbit_camera_retarget(Orbit_Camera& orbit, Vector3 target);
Camera3D orbit_camera_build(const Orbit_Camera& orbit);

} // namespace sdk

#pragma once

#include "raylib.h"

namespace sdk {

struct Orbit_Config {
    float mouse_sensitivity;
    float wheel_speed;
    float min_pitch;
    float max_pitch;
    float min_distance;
    float max_distance;
    float fovy;
};

struct Orbit_Camera {
    Vector3 target;
    float yaw;
    float pitch;
    float distance;
    Camera3D camera;
};

struct Cursor_Capture {
    int wants_capture;
    int is_captured;
    int was_focused;
    int suppress_delta;
};

void cursor_capture_update(Cursor_Capture& cursor, int wants_capture, int window_focused);
void orbit_camera_init(Orbit_Camera& orbit, Vector3 target, float yaw, float pitch, float distance, float fovy);
void orbit_camera_update(Orbit_Camera& orbit, const Orbit_Config& config, Vector2 mouse_delta, float wheel, int rotate_from_mouse);
void orbit_camera_retarget(Orbit_Camera& orbit, Vector3 target);

}

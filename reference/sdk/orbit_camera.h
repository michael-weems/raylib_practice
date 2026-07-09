#ifndef REFERENCE_SDK_ORBIT_CAMERA_H
#define REFERENCE_SDK_ORBIT_CAMERA_H

#include "raylib.h"

namespace sdk {

struct Orbit_Camera_Config {
    float radians_per_mouse_pixel;
    float world_units_per_wheel_step;

    float minimum_pitch;
    float maximum_pitch;

    float minimum_distance;
    float maximum_distance;

    Vector3 up;
    float fovy;
    CameraProjection projection;
};

struct Orbit_Camera_State {
    Vector3 target;
    float yaw;
    float pitch;
    float distance;

    int wants_cursor_captured;
    int is_cursor_captured;
    int suppress_mouse_delta;
    int was_window_focused;
};

struct Orbit_Camera_Input {
    Vector2 mouse_delta;
    float wheel_delta;
    int capture_toggle_pressed;
    int is_window_focused;
};

enum Orbit_Camera_Update_Result {
    ORBIT_CAMERA_UPDATE_SUCCESS = 0,
    ORBIT_CAMERA_INVALID_CONFIG
};

void orbit_camera_request_capture(Orbit_Camera_State& state, int should_capture);
Orbit_Camera_Update_Result orbit_camera_update(
    const Orbit_Camera_Config& config,
    const Orbit_Camera_Input& input,
    Orbit_Camera_State& state,
    Camera3D& camera);

} // namespace sdk

#endif // REFERENCE_SDK_ORBIT_CAMERA_H

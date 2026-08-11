#ifndef SDK_ORBIT_CAMERA_H
#define SDK_ORBIT_CAMERA_H

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
   float distance;
   float pitch;
   float yaw;
};

struct Cursor_Capture_State {
   bool wants_cursor_captured;
   bool is_cursor_captured;
};

struct Orbit_Camera_Input {
   Vector2 mouse_delta;
   float wheel_delta;
   bool rotate_from_mouse;
};

struct Cursor_Capture_Input {
   bool capture_toggle_pressed;
   bool is_window_focused;
};

struct Orbit_Camera_Derived {
   Vector3 position_offset;
   Vector3 forward;
   Vector3 right;
   Vector3 view_up;
};

// Reconciles logical capture with Raylib's physical cursor state and returns
// true only when this frame's sampled mouse delta is safe to use for rotation.
bool cursor_capture_update(const Cursor_Capture_Input& input, Cursor_Capture_State& state);

// Validates immutable orbit policy and camera description once during setup.
bool orbit_camera_config_is_valid(const Orbit_Camera_Config& config);

// Applies sampled controls to persistent orientation and distance only.
void orbit_camera_apply_input(const Orbit_Camera_Config& config, const Orbit_Camera_Input& input, Orbit_Camera_State& state);

// Calculates one target-independent offset and orthonormal basis for this frame.
void orbit_camera_derive(const Orbit_Camera_Config& config, const Orbit_Camera_State& state, Orbit_Camera_Derived& derived);

// Places the derived orbit around the final application-owned target.
void orbit_camera_build(const Orbit_Camera_Config& config, const Orbit_Camera_Derived& derived, Vector3 target, Camera3D& camera);

} // namespace sdk

#endif // SDK_ORBIT_CAMERA_H

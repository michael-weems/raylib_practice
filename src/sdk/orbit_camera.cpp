#include "sdk/orbit_camera.h"

#include <cmath>

#include "raylib.h"
#include "raymath.h"

namespace sdk {

bool cursor_capture_update(const Cursor_Capture_Input& input, Cursor_Capture_State& state) {
   if (input.is_window_focused && input.capture_toggle_pressed) {
      state.wants_cursor_captured = !state.wants_cursor_captured;
   }

   bool should_capture_cursor{ input.is_window_focused && state.wants_cursor_captured };
   bool physical_capture_changed{ false };

   if (should_capture_cursor && !state.is_cursor_captured) {
      DisableCursor();
      state.is_cursor_captured = true;
      physical_capture_changed = true;
   } else if (!should_capture_cursor && state.is_cursor_captured) {
      EnableCursor();
      state.is_cursor_captured = false;
      physical_capture_changed = true;
   }

   return state.is_cursor_captured && !physical_capture_changed;
}

bool orbit_camera_config_is_valid(const Orbit_Camera_Config& config) {
   if (config.radians_per_mouse_pixel <= 0.0f) return false;
   if (config.world_units_per_wheel_step <= 0.0f) return false;
   if (config.minimum_pitch <= -PI / 2.0f) return false;
   if (config.maximum_pitch >= PI / 2.0f) return false;
   if (config.minimum_pitch >= config.maximum_pitch) return false;
   if (config.minimum_distance <= 0.0f) return false;
   if (config.minimum_distance >= config.maximum_distance) return false;
   if (config.up.x == 0.0f && config.up.y == 0.0f && config.up.z == 0.0f) return false;
   if (config.fovy <= 0.0f) return false;
   if (config.projection != CAMERA_PERSPECTIVE && config.projection != CAMERA_ORTHOGRAPHIC) return false;

   return true;
}

void orbit_camera_apply_input(const Orbit_Camera_Config& config, const Orbit_Camera_Input& input, Orbit_Camera_State& state) {

   if (input.rotate_from_mouse) {
      state.yaw = state.yaw + (input.mouse_delta.x * config.radians_per_mouse_pixel);
      state.pitch = state.pitch + (input.mouse_delta.y * config.radians_per_mouse_pixel);
   }

   state.yaw = std::remainder(state.yaw, 2.0f * PI);

   if (state.pitch < config.minimum_pitch) state.pitch = config.minimum_pitch;
   if (state.pitch > config.maximum_pitch) state.pitch = config.maximum_pitch;

   state.distance = state.distance - (input.wheel_delta * config.world_units_per_wheel_step);
   if (state.distance < config.minimum_distance) state.distance = config.minimum_distance;
   if (state.distance > config.maximum_distance) state.distance = config.maximum_distance;
}

void orbit_camera_derive(const Orbit_Camera_Config& config, const Orbit_Camera_State& state, Orbit_Camera_Derived& derived) {
   // Pitch remains meaningfully inside +/-90 degrees. At extreme near-vertical
   // pitch, adding a tiny horizontal offset to a large float target can round
   // that offset away and make the final Camera3D basis degenerate.
   double horizontal_radius{ state.distance * std::cos(state.pitch) };
   double vertical_offset{ state.distance * std::sin(state.pitch) };

   double x_yaw_offset{ horizontal_radius * std::sin(state.yaw) };
   double z_yaw_offset{ horizontal_radius * std::cos(state.yaw) };

   derived.position_offset = Vector3{
      static_cast<float>(x_yaw_offset),
      static_cast<float>(vertical_offset),
      static_cast<float>(z_yaw_offset)
   };
   derived.forward = Vector3Normalize(Vector3Negate(derived.position_offset));
   derived.right = Vector3Normalize(Vector3CrossProduct(derived.forward, config.up));
   derived.view_up = Vector3Normalize(Vector3CrossProduct(derived.right, derived.forward));
}

void orbit_camera_build(const Orbit_Camera_Config& config, const Orbit_Camera_Derived& derived, Vector3 target, Camera3D& camera) {
   camera.target = target;
   camera.position = Vector3Add(camera.target, derived.position_offset);
   camera.up = config.up;
   camera.fovy = config.fovy;
   camera.projection = config.projection;
}

} // namespace sdk


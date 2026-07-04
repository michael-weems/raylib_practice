#include "sdk/orbit_camera.h"

#include <cmath>

#include "raylib.h"

namespace sdk {

Orbit_Camera_Update_Result orbit_camera_update(const Orbit_Camera_Config& config, const Orbit_Camera_Input& input, Orbit_Camera_State& state, Camera3D& camera) {

   if (config.radians_per_mouse_pixel <= 0) return ORBIT_CAMERA_INVALID_CONFIG;
   if (config.world_units_per_wheel_step <= 0) return ORBIT_CAMERA_INVALID_CONFIG;
   if (config.minimum_pitch <= -PI/2.0f) return ORBIT_CAMERA_INVALID_CONFIG;
   if (config.maximum_pitch >= PI/2.0f) return ORBIT_CAMERA_INVALID_CONFIG;
   if (config.minimum_pitch >= config.maximum_pitch) return ORBIT_CAMERA_INVALID_CONFIG;

   if (config.minimum_distance <= 0) return ORBIT_CAMERA_INVALID_CONFIG;
   if (config.minimum_distance >= config.maximum_distance) return ORBIT_CAMERA_INVALID_CONFIG;

   if (config.up.x == 0 && config.up.y == 0 && config.up.z == 0) return ORBIT_CAMERA_INVALID_CONFIG;
   if (config.fovy <= 0) return ORBIT_CAMERA_INVALID_CONFIG;
   if (config.projection != CAMERA_PERSPECTIVE && config.projection != CAMERA_ORTHOGRAPHIC) return ORBIT_CAMERA_INVALID_CONFIG;

   if (!input.is_window_focused && state.was_window_focused) {
      EnableCursor();
   }

   bool suppress_next_mouse_delta{ false };
   if (input.is_window_focused && !state.was_window_focused) {
      if (state.wants_cursor_captured) {
         DisableCursor();
         suppress_next_mouse_delta = true;
      }
   }
   if (input.is_window_focused && input.capture_toggle_pressed) { 
      state.wants_cursor_captured = !state.wants_cursor_captured;
      if (state.wants_cursor_captured) {
         DisableCursor();
         suppress_next_mouse_delta = true;
      } else { 
         EnableCursor();
      }
   }

   if (input.is_window_focused && state.wants_cursor_captured && !suppress_next_mouse_delta) {
      if (state.suppress_mouse_delta) {
         state.suppress_mouse_delta = false;
      } else {
         state.yaw = state.yaw + (input.mouse_delta.x * config.radians_per_mouse_pixel);
         state.pitch = state.pitch + (input.mouse_delta.y * config.radians_per_mouse_pixel);
      }
   }

   if (state.pitch < config.minimum_pitch) state.pitch = config.minimum_pitch;
   if (state.pitch > config.maximum_pitch) state.pitch = config.maximum_pitch;

   state.distance = state.distance - (input.wheel_delta * config.world_units_per_wheel_step);
   if (state.distance < config.minimum_distance) state.distance = config.minimum_distance;
   if (state.distance > config.maximum_distance) state.distance = config.maximum_distance;

   double horizontal_radius{ state.distance * std::cos(state.pitch) };
   double vertical_offset{ state.distance * std::sin(state.pitch) };

   double x_yaw_offset{ horizontal_radius * std::sin(state.yaw) };
   double z_yaw_offset{ horizontal_radius * std::cos(state.yaw) };

   camera.up       = config.up;
   camera.fovy     = config.fovy;
   camera.projection = config.projection;

   camera.target = state.target;
   camera.position = Vector3{ camera.target.x + (float)x_yaw_offset, camera.target.y + (float)vertical_offset, camera.target.z + (float)(z_yaw_offset) };

   state.was_window_focused   = input.is_window_focused;
   state.suppress_mouse_delta = suppress_next_mouse_delta;

   return ORBIT_CAMERA_UPDATE_SUCCESS;
}

} // namespace sdk


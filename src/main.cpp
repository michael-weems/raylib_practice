#include "raylib.h"
#include "raymath.h"

#include "sdk/runtime.h"
#include "sdk/orbit_camera.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <cmath>

int main() { 
   sdk::Runtime_Config config = {};
   sdk::Runtime_State runtime = {};

   config.title = "checkpoint 5";

   sdk::Runtime_Result result = sdk::runtime_init(runtime, config);
   if (result != sdk::RUNTIME_SUCCESS) {
      switch (result) {
      case sdk::RUNTIME_ERROR_ALREADY_INITIALIZED:
         std::cerr << "ERR: ALREADY INITIALIZED" << std::endl;
         break;
      case sdk::RUNTIME_ERROR_WINDOW_INITIALIZATION:
         std::cerr << "ERR: WINDOW INITIALIZATION" << std::endl;
         break;
      }

      return result;
   }

   sdk::Orbit_Camera_Config camera_config = {};
   camera_config.radians_per_mouse_pixel    = 0.005f;
   camera_config.world_units_per_wheel_step = 0.5f;
   camera_config.minimum_pitch    = -85.0f * DEG2RAD;
   camera_config.maximum_pitch    = 85.0f * DEG2RAD;
   camera_config.minimum_distance = 3.0f;
   camera_config.maximum_distance = 30.0f;
   camera_config.up   = Vector3{0, 1, 0};
   camera_config.fovy = 70;
   camera_config.projection = CAMERA_PERSPECTIVE;


   sdk::Orbit_Camera_State camera_state = {};
   camera_state.distance = 10.0f;
   camera_state.pitch = 30 * DEG2RAD;
   camera_state.yaw   = 0.0f;
   camera_state.wants_cursor_captured = true;
   camera_state.suppress_mouse_delta  = true;
   camera_state.was_window_focused    = false;
   camera_state.target = Vector3{ 0, 0, 0 };

   char buffer[20] = { 0 };
   char camera_overlay_buffer[100] = { 0 };

   auto startup_time = std::chrono::steady_clock::now();

   float x = 0.0f;
   float y = 0.0f;
   int font_size = 16;

   while (!WindowShouldClose()) {
      auto frame_time = std::chrono::steady_clock::now();
      auto diff = frame_time - startup_time;
      double total_time = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() / 1000.0;

      float dt = GetFrameTime();

      x += dt * 20.0f;
      y += dt * 20.0f;

      if ((int)x >= config.screen_width) x = 0.0f;
      if ((int)y >= config.screen_height) y = 0.0f;

      sdk::Orbit_Camera_Input camera_input = {}; 
      camera_input.is_window_focused = IsWindowFocused();
      camera_input.mouse_delta = GetMouseDelta();
      camera_input.wheel_delta = GetMouseWheelMove();
      camera_input.capture_toggle_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

      Camera3D camera = {};
      
      sdk::Orbit_Camera_Update_Result camera_update{ sdk::orbit_camera_update(camera_config, camera_input, camera_state, camera) };
      if (camera_update != sdk::ORBIT_CAMERA_UPDATE_SUCCESS) {
         std::cerr << "ERR: CAMERA UPDATE" << std::endl;
         break;
      }

      BeginDrawing();
         ClearBackground(BLACK);

         BeginMode3D(camera);
            DrawGrid(10, 1);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{10, 0, 0}, BLUE);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{0, 10, 0}, RED);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{0, 0, 10}, GREEN);

            DrawCubeV(Vector3{0, 0, 0}, Vector3{2, 2, 2}, YELLOW);
            DrawCubeWiresV(Vector3{0, 0, 0}, Vector3{2, 2, 2}, MAGENTA);
         EndMode3D();

         std::snprintf(buffer, sizeof(buffer), "seconds: %.2f", total_time);
         DrawText(buffer, (int)x, (int)y, font_size, RAYWHITE);

         int y_offset = 5;
         int x_offset = 5;
         int fps = GetFPS();
         DrawText(TextFormat("FPS: %i", fps), x_offset, y_offset, font_size, YELLOW);
         y_offset += font_size;
         DrawText("Ya Boi", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;
         DrawText("ESC: Exit", x_offset, y_offset, font_size, RAYWHITE);

         y_offset += font_size;
         std::snprintf(camera_overlay_buffer, sizeof(camera_overlay_buffer), "CAMERA: target (%.2f,%.2f,%.2f) yaw (%.2f) pitch (%.2f) distance (%.2f) capture (%d)", camera.target.x, camera.target.y, camera.target.z, camera_state.yaw, camera_state.pitch, camera_state.distance, camera_state.wants_cursor_captured);
         DrawText(camera_overlay_buffer, x_offset, y_offset, font_size, RAYWHITE);

         if (runtime.used_defaults) {
            y_offset += font_size;
            DrawText("USED DEFAULT CONFIG VALUES", x_offset, y_offset, font_size, YELLOW);
         }
      EndDrawing();
   }

   sdk::runtime_shutdown(runtime);

   return 0;
}

#include "raylib.h"
#include "raymath.h"

#include "sdk/runtime.h"
#include "sdk/orbit_camera.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <cmath>

int MAX_X{ 5 };
int MAX_Y{ 7 };
int MAX_Z{ 8 };
Vector3 CUBE_SIZE{ 2, 2, 2 };
float CUBE_SPACING{ 5.0f };

struct Coord {
   int x;
   int y;
   int z;
};
struct Dim {
   int x;
   int y;
   int z;
};
struct Handle {
   uint32_t id;
};

Dim MAX_DIM{ MAX_X, MAX_Y, MAX_Z };

static Handle max_handle(Dim dim) {
   return Handle{ static_cast<uint32_t>(dim.x * dim.y * dim.z) };
}

// grid-coordinates to handle
static Handle get_handle(Coord coords, Dim widths) {
   if (coords.x < 0 || coords.x >= widths.x) return Handle{ 0 };
   if (coords.y < 0 || coords.y >= widths.y) return Handle{ 0 };
   if (coords.z < 0 || coords.z >= widths.z) return Handle{ 0 };

   return Handle{ static_cast<uint32_t>(coords.x + (coords.y * widths.x) + (coords.z * widths.x * widths.y)) + 1 };
}

// handle to grid-coordinates
static bool get_coordinates(Handle handle, Dim widths, Coord& out) {
   if (handle.id == 0) return false;
   if (handle.id > max_handle(widths)) return false;

   int index{ static_cast<int>(handle.id) - 1 };
   out.z = (index / (widths.x * widths.y));

   int remainder{ index % (widths.x * widths.y) };
   out.y = remainder / widths.x;
   out.x = remainder % static_cast<int>(widths.x);
   
   return true;
}

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

   float text_x = 0.0f;
   float text_y = 0.0f;
   int font_size = 16;

   while (!WindowShouldClose()) {
      auto frame_time = std::chrono::steady_clock::now();
      auto diff = frame_time - startup_time;
      double total_time = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() / 1000.0;

      float dt = GetFrameTime();

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

      text_x += dt * 20.0f;
      text_y += dt * 20.0f;

      if ((int)text_x >= config.screen_width) text_x = 0.0f;
      if ((int)text_y >= config.screen_height) text_y = 0.0f;

      Coord selected{1,3,4};
      Handle handle{ get_handle(selected, MAX_DIM) };
      Coord c = {};
      bool is_selected_valid{ get_coordinates(handle, MAX_DIM, c) };

      BeginDrawing();
         ClearBackground(BLACK);

         BeginMode3D(camera);
            DrawGrid(10, 1);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{10, 0, 0}, BLUE);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{0, 10, 0}, RED);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{0, 0, 10}, GREEN);

            for (int z{ 0 }; z < MAX_Z; ++z) {
               for (int y{ 0 }; y < MAX_Y; ++y) {
                  for (int x{ 0 }; x < MAX_X; ++x) {
                     // 0 1 2 3 4
                     //     ^
                     // 0 1 2 3 4 5
                     //       ^

                     Vector3 p = {};
                     p.x = CUBE_SPACING * (static_cast<float>(x - (MAX_X / 2)) + (static_cast<float>((MAX_X & 1) == 0) * 0.5f));
                     p.y = CUBE_SPACING * (static_cast<float>(y - (MAX_Y / 2)) + (static_cast<float>((MAX_Y & 1) == 0) * 0.5f));
                     p.z = CUBE_SPACING * (static_cast<float>(z - (MAX_Z / 2)) + (static_cast<float>((MAX_Z & 1) == 0) * 0.5f));

                     if (is_selected_valid && handle.id == get_handle(Coord{x,y,z}, MAX_DIM).id) {
                        DrawCubeV(p, CUBE_SIZE, GREEN);
                     } else {
                        DrawCubeV(p, CUBE_SIZE, YELLOW);
                     }

                     DrawCubeWiresV(p, CUBE_SIZE, MAGENTA);
                  }
               }
            }

         EndMode3D();

         std::snprintf(buffer, sizeof(buffer), "seconds: %.2f", total_time);
         DrawText(buffer, (int)text_x, (int)text_y, font_size, RAYWHITE);

         int y_offset = 5;
         int x_offset = 5;
         int fps = GetFPS();
         DrawText(TextFormat("FPS: %i", fps), x_offset, y_offset, font_size, YELLOW);
         y_offset += font_size;
         DrawText("Ya Boi", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;
         DrawText("ESC: Exit", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         if (is_selected_valid) {
            DrawText(TextFormat("CUBE: handle = %u | x = %i  y = %i  z = %i", handle.id, c.x, c.y, c.z), x_offset, y_offset, font_size, RAYWHITE);
         } else { 
            DrawText("CUBE: handle = FAIL");
         }

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

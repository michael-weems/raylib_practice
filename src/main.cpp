#include "raylib.h"
#include "raymath.h"

#include "sdk/runtime.h"
#include "sdk/orbit_camera.h"

#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <cmath>

// Modern C++ using syntax (Recommended)
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

f64 X_MIN{ -0.5f };
f64 X_MAX{  7.0f };
f64 X_STEP{ 0.5f };
u32 X_STEP_COUNT{ static_cast<u32>(std::llround((X_MAX - X_MIN) / X_STEP)) + 1 };

f64 Y_MIN{ -4.0f };
f64 Y_MAX{  4.0f };
f64 Y_STEP{ 0.5f };
u32 Y_STEP_COUNT{ static_cast<u32>(std::llround((Y_MAX - Y_MIN) / Y_STEP)) + 1 };

f64 Z_MIN{ -5.0f };
f64 Z_MAX{  5.0f };
f64 Z_STEP{ 0.5f };
u32 Z_STEP_COUNT{ static_cast<u32>(std::llround((Z_MAX - Z_MIN) / Z_STEP)) + 1 };

Vector3 CUBE_SIZE{ 2, 2, 2 };
f32 CUBE_SPACING{ 5.0f };

struct Cube_Index {
   u32 x;
   u32 y;
   u32 z;
};
struct Cube_Count {
   u32 x;
   u32 y;
   u32 z;
};
struct Cube_Handle {
   u32 id;
};

struct Cube_Position {
   f32 x;
   f32 y;
   f32 z;
};

enum Value : u8 {
   VALUE_A = 0,
   VALUE_B,
   VALUE_C,
   VALUE_D,
   VALUE_COUNT
};

struct Cube_Styling {
   Color fill_color;
   Color wire_color;
};
struct Cube_Palette {
   Cube_Styling styles[VALUE_COUNT];
};
struct Cube_Palette_Handle {
   u32 id;
};

Cube_Count CUBE_COUNT{ X_STEP_COUNT, Y_STEP_COUNT, Z_STEP_COUNT };
u32 CUBE_TOTAL_COUNT{ X_STEP_COUNT * Y_STEP_COUNT * Z_STEP_COUNT };

const u32 PALETTE_1{ 0 };
const u32 PALETTE_2{ 1 };
const u32 PALETTE_3{ 2 };

static Cube_Handle max_handle() {
   return Cube_Handle{ CUBE_TOTAL_COUNT - 1 };
}

// grid-coordinates to handle
static Cube_Handle get_handle(Cube_Index i, Cube_Count c) {
   if (i.x >= c.x) return Cube_Handle{ 0 };
   if (i.y >= c.y) return Cube_Handle{ 0 };
   if (i.z >= c.z) return Cube_Handle{ 0 };

   return Cube_Handle{ static_cast<u32>(i.x + (i.y * c.x) + (i.z * c.x * c.y)) };
}

// handle to grid-coordinates
static bool get_coordinates(Cube_Handle handle, Cube_Count widths, Cube_Index& out) {
   if (handle.id > max_handle().id) return false;

   u32 index{ static_cast<u32>(handle.id) };
   out.z = (index / (widths.x * widths.y));

   u32 remainder{ index % (widths.x * widths.y) };
   out.y = remainder / widths.x;
   out.x = remainder % static_cast<u32>(widths.x);
   
   return true;
}


inline Vector3 get_world_vector3(Cube_Index coords, Cube_Count widths, f32 spacing) {
   return Vector3{
      spacing * (static_cast<f32>(coords.x) - ((static_cast<f32>(widths.x) - 1.0f) * 0.5f)),
      spacing * (static_cast<f32>(coords.y) - ((static_cast<f32>(widths.y) - 1.0f) * 0.5f)),
      spacing * (static_cast<f32>(coords.z) - ((static_cast<f32>(widths.z) - 1.0f) * 0.5f))
   };
}


static const char * get_value_string(Value v) {
   switch (v) {
   case VALUE_A: return "A\0";
   case VALUE_B: return "B\0";
   case VALUE_C: return "C\0";
   case VALUE_D: return "D\0";
   default:      return "OOPS\0";
   }
}

static u32 hash32(u32 a) {
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    return a;
}

static u32 hash_coord(Cube_Index coordinate) {
   u32 mixed_coordinate = 0x9E3779B9u;

   mixed_coordinate ^= static_cast<u32>(coordinate.x) * 0x85EBCA6Bu;
   mixed_coordinate ^= static_cast<u32>(coordinate.y) * 0xC2B2AE35u;
   mixed_coordinate ^= static_cast<u32>(coordinate.z) * 0x27D4EB2Fu;

   return hash32(mixed_coordinate);
}

enum Test_Handles {  
   ACTUAL_SELECTED = 0, 
   TEST_HANDLE_ONE,
   TEST_HANDLE_TWO
};

inline i32 square_i32(i32 value) {
   return value * value;
}

i32 main() { 
   Value* values = static_cast<Value*>(std::malloc((CUBE_TOTAL_COUNT) * sizeof(Value)));
   if (values == nullptr) {
      std::cerr << "ERR: ALLOC VALUES" << std::endl;
      return 1;
   }

   for (u32 z{ 0 }; z < Z_STEP_COUNT; ++z) {
      for (u32 y{ 0 }; y < Y_STEP_COUNT; ++y) {
         for (u32 x{ 0 }; x < X_STEP_COUNT; ++x) {
            Cube_Handle handle{ get_handle({x,y,z}, CUBE_COUNT) };
            values[handle.id] = static_cast<Value>((hash_coord({x,y,z}) & 3u));
         }
      }
   }

   Cube_Palette palettes[3] = { };
   palettes[0].styles[VALUE_A] = {BLUE, RAYWHITE};
   palettes[0].styles[VALUE_B] = {RED, RAYWHITE};
   palettes[0].styles[VALUE_C] = {GREEN, RAYWHITE};
   palettes[0].styles[VALUE_D] = {Color{255, 255, 255, 153}, RAYWHITE};

   palettes[1].styles[VALUE_A] = {YELLOW, RAYWHITE};
   palettes[1].styles[VALUE_B] = {ORANGE, RAYWHITE};
   palettes[1].styles[VALUE_C] = {MAROON, RAYWHITE};
   palettes[1].styles[VALUE_D] = {Color{255, 255, 255, 153}, RAYWHITE};

   palettes[2].styles[VALUE_A] = {BEIGE, GREEN};
   palettes[2].styles[VALUE_B] = {BROWN, GREEN};
   palettes[2].styles[VALUE_C] = {DARKBROWN, GOLD};
   palettes[2].styles[VALUE_D] = {Color{255, 255, 255, 153}, GOLD};

   sdk::Runtime_Config config = {};
   sdk::Runtime_State runtime = {};

   config.title = "yaboi";

   sdk::Runtime_Result result = sdk::runtime_init(runtime, config);
   if (result != sdk::RUNTIME_SUCCESS) {
      switch (result) {
      case sdk::RUNTIME_ERROR_ALREADY_INITIALIZED:
         std::cerr << "ERR: ALREADY INITIALIZED" << std::endl;
         break;
      case sdk::RUNTIME_ERROR_WINDOW_INITIALIZATION:
         std::cerr << "ERR: WINDOW INITIALIZATION" << std::endl;
         break;
      default:
         break;
      }

      std::free(values);
      values = nullptr;
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

   f32 text_x = 0.0f;
   f32 text_y = 0.0f;
   i32 font_size = 16;

   Cube_Palette_Handle palette{ 1 };

   Cube_Index c{};
   Cube_Handle selected_handle = get_handle(c, CUBE_COUNT);

   while (!WindowShouldClose()) {
      auto frame_time = std::chrono::steady_clock::now();
      auto diff = frame_time - startup_time;
      double total_time = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() / 1000.0;

      f32 dt = GetFrameTime();
      
      bool is_selected_valid{ get_coordinates(selected_handle, CUBE_COUNT, c) };
      
      switch (GetKeyPressed()) { 
      case KEY_ONE:   palette.id = PALETTE_1; break;
      case KEY_TWO:   palette.id = PALETTE_2; break;
      case KEY_THREE: palette.id = PALETTE_3; break;
      case KEY_H:
         if (c.x != 0) --c.x;
         if (c.x < 0) c.x = 0;
         break;
      case KEY_L:
         ++c.x;
         if (c.x >= X_STEP_COUNT) c.x = X_STEP_COUNT - 1;
         break;
      case KEY_K:
         ++c.y;
         if (c.y >= Y_STEP_COUNT) c.y = Y_STEP_COUNT - 1;
         break;
      case KEY_J:
         if (c.y != 0) --c.y;
         if (c.y < 0) c.y = 0;
         break;
      case KEY_I:
         ++c.z;
         if (c.z >= Z_STEP_COUNT) c.z = Z_STEP_COUNT - 1;
         break;
      case KEY_U:
         if (c.z != 0) --c.z;
         if (c.z < 0) c.z = 0;
         break;
      default: break;
      }

      selected_handle = get_handle(c, CUBE_COUNT);

      is_selected_valid = get_coordinates(selected_handle, CUBE_COUNT, c);

      text_x += dt * 20.0f;
      text_y += dt * 20.0f;

      if ((i32)text_x >= config.screen_width) text_x = 0.0f;
      if ((i32)text_y >= config.screen_height) text_y = 0.0f;

      Value selected_value{ VALUE_A };

      Vector3 world_center{ get_world_vector3(c, CUBE_COUNT, CUBE_SPACING) };
      camera_state.target = world_center;

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
      
      i32 bound{ 3 };

      i32 lx{ static_cast<i32>(c.x) - bound };
      u32 ux{ c.x + bound };
      if (lx < 0) lx = 0;
      if (ux >= X_STEP_COUNT) ux = X_STEP_COUNT - 1;

      i32 ly{ static_cast<i32>(c.y) - bound };
      u32 uy{ c.y + bound };
      if (ly < 0) ly = 0;
      if (uy >= Y_STEP_COUNT) uy = Y_STEP_COUNT - 1;

      i32 lz{ static_cast<i32>(c.z) - bound };
      u32 uz{ c.z + bound };
      if (lz < 0) lz = 0;
      if (uz >= Z_STEP_COUNT) uz = Z_STEP_COUNT - 1;

      u32 submitted_cubes{ 0 };

      BeginDrawing();
         ClearBackground(BLACK);

         BeginMode3D(camera);
            DrawGrid(10, 1);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{10, 0, 0}, BLUE);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{0, 10, 0}, RED);
            DrawLine3D(Vector3{0, 0, 0}, Vector3{0, 0, 10}, GREEN);

            const Cube_Palette& styles{ palettes[palette.id] };

            i32 radius_squared{ square_i32(bound) };

            for (u32 z{ static_cast<u32>(lz) }; z <= uz; ++z) {
               i32 dz{ square_i32(static_cast<i32>(z) - static_cast<i32>(c.z)) };
               for (u32 y{ static_cast<u32>(ly) }; y <= uy; ++y) {
                  i32 dy{ square_i32(static_cast<i32>(y) - static_cast<i32>(c.y)) };
                  for (u32 x{ static_cast<u32>(lx) }; x <= ux; ++x) {
                     i32 dx{ square_i32(static_cast<i32>(x) - static_cast<i32>(c.x)) };

                     i32 distance_squared{ dx + dy + dz };
                     if (distance_squared > radius_squared) continue;

                     ++submitted_cubes;

                     // 0 1 2 3 4
                     //     ^
                     // 0 1 2 3 4 5
                     //       ^

                     Cube_Handle cube_handle{ get_handle({x,y,z}, CUBE_COUNT) };
                     Value cube_value{ values[cube_handle.id] };

                     Vector3 p{ get_world_vector3({x,y,z}, CUBE_COUNT, CUBE_SPACING) }; 

                     DrawCubeV(p, CUBE_SIZE, styles.styles[cube_value].fill_color);
                     if (is_selected_valid && selected_handle.id == cube_handle.id) {
                        DrawCubeWiresV(p, CUBE_SIZE, LIME);
                     } else {
                        DrawCubeWiresV(p, CUBE_SIZE, styles.styles[cube_value].wire_color);
                     }

                  }
               }
            }

         EndMode3D();

         std::snprintf(buffer, sizeof(buffer), "seconds: %.2f", total_time);
         DrawText(buffer, (i32)text_x, (i32)text_y, font_size, RAYWHITE);

         i32 y_offset = 5;
         i32 x_offset = 5;
         i32 fps = GetFPS();
         DrawText(TextFormat("FPS: %i", fps), x_offset, y_offset, font_size, YELLOW);
         y_offset += font_size;
         DrawText("Ya Boi", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;
         DrawText("ESC: Exit", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;
         DrawText(TextFormat("Palette: %i", palette.id), x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         i32 cubes_tested{ (static_cast<i32>(ux)-lx + 1) * (static_cast<i32>(uy)-ly + 1) * (static_cast<i32>(uz)-lz + 1) };
         DrawText(TextFormat("Tested Cubes: %i", cubes_tested), x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         DrawText(TextFormat("Submitted Cubes: %u", submitted_cubes), x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         if (is_selected_valid) {
            selected_value = values[selected_handle.id];
            DrawText(TextFormat("CUBE: handle = %u  value = %s | x = %i  y = %i  z = %i", selected_handle.id, get_value_string(selected_value), c.x, c.y, c.z), x_offset, y_offset, font_size, RAYWHITE);
         } else { 
            DrawText("CUBE: handle = FAIL", x_offset, y_offset, font_size, RAYWHITE);
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

   std::free(values);
   values = nullptr;

   sdk::runtime_shutdown(runtime);

   return 0;
}

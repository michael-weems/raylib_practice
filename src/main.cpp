#include "raylib.h"
#include "raymath.h"

#include "sdk/runtime.h"
#include "sdk/orbit_camera.h"

#include <cstdlib>
#include <cstdint>
#include <iostream>

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

struct Sampled_Axis {
   f64 minimum;
   f64 step;
   u32 sample_count;
};

struct Sampled_Domain {
   Sampled_Axis x;
   Sampled_Axis y;
   Sampled_Axis z;
};

struct Sampled_Coordinate {
   f64 x;
   f64 y;
   f64 z;
};

const Sampled_Domain CUBE_DOMAIN{
   Sampled_Axis{ -0.5, 0.5, 16 },
   Sampled_Axis{ -4.0, 0.5, 17 },
   Sampled_Axis{ -5.0, 0.5, 21 }
};

const u32 X_STEP_COUNT{ CUBE_DOMAIN.x.sample_count };
const u32 Y_STEP_COUNT{ CUBE_DOMAIN.y.sample_count };
const u32 Z_STEP_COUNT{ CUBE_DOMAIN.z.sample_count };

const Vector3 CUBE_SIZE{ 2, 2, 2 };
const f32 CUBE_SPACING{ 5.0f };

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

static Sampled_Coordinate get_sampled_coordinate(Cube_Index index, const Sampled_Domain& domain) {
   return Sampled_Coordinate{
      domain.x.minimum + static_cast<f64>(index.x) * domain.x.step,
      domain.y.minimum + static_cast<f64>(index.y) * domain.y.step,
      domain.z.minimum + static_cast<f64>(index.z) * domain.z.step
   };
}

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

const Cube_Count CUBE_COUNT{ X_STEP_COUNT, Y_STEP_COUNT, Z_STEP_COUNT };
const u32 CUBE_TOTAL_COUNT{ X_STEP_COUNT * Y_STEP_COUNT * Z_STEP_COUNT };

enum Palette_Id : u32 {
   PALETTE_1 = 0,
   PALETTE_2,
   PALETTE_3,
   PALETTE_COUNT
};

// grid-coordinates to handle
static Cube_Handle get_handle(Cube_Index i, Cube_Count c) {
   return Cube_Handle{ static_cast<u32>(i.x + (i.y * c.x) + (i.z * c.x * c.y)) };
}


inline Vector3 get_world_vector3(Cube_Index coords, Cube_Count widths, f32 spacing) {
   // Sampled values describe the data domain. Centered indices and spacing
   // independently describe where those samples are rendered in world space.
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

inline i32 square_i32(i32 value) {
   return value * value;
}

inline f32 abs_f32(f32 value) {
   if (value < 0.0f) {
      return -value;
   }
   return value;
}

enum Move {
   MOVE_NONE = 0,
   MOVE_LEFT,
   MOVE_RIGHT,
   MOVE_FORWARD,
   MOVE_BACKWARD,
   MOVE_UP,
   MOVE_DOWN
};

struct Frame_Input {
   Vector2 mouse_delta;
   f32 mouse_wheel;
   sdk::Cursor_Capture_Input cursor;
   Move movement;
   Cube_Palette_Handle requested_palette;
   bool palette_changed;
};

static i32 clamp_i32(i32 value, i32 minimum, i32 maximum) {
   if (value < minimum) return minimum;
   if (value > maximum) return maximum;
   return value;
}

static Frame_Input poll_frame_input() {
   Frame_Input input = {};

   input.mouse_delta = GetMouseDelta();
   input.mouse_wheel = GetMouseWheelMove();
   input.cursor.is_window_focused = IsWindowFocused();
   input.cursor.capture_toggle_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

   if (IsKeyPressed(KEY_ONE)) {
      input.requested_palette.id = PALETTE_1;
      input.palette_changed = true;
   } else if (IsKeyPressed(KEY_TWO)) {
      input.requested_palette.id = PALETTE_2;
      input.palette_changed = true;
   } else if (IsKeyPressed(KEY_THREE)) {
      input.requested_palette.id = PALETTE_3;
      input.palette_changed = true;
   }

   if (IsKeyPressed(KEY_H)) {
      input.movement = MOVE_LEFT;
   } else if (IsKeyPressed(KEY_L)) {
      input.movement = MOVE_RIGHT;
   } else if (IsKeyPressed(KEY_U)) {
      input.movement = MOVE_FORWARD;
   } else if (IsKeyPressed(KEY_I)) {
      input.movement = MOVE_BACKWARD;
   } else if (IsKeyPressed(KEY_K)) {
      input.movement = MOVE_UP;
   } else if (IsKeyPressed(KEY_J)) {
      input.movement = MOVE_DOWN;
   }

   return input;
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

   Cube_Palette palettes[PALETTE_COUNT] = { };
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
   // This application keeps a deliberate margin from vertical so the
   // horizontal orbit basis remains meaningful across the field's extent.
   camera_config.minimum_pitch    = -85.0f * DEG2RAD;
   camera_config.maximum_pitch    = 85.0f * DEG2RAD;
   camera_config.minimum_distance = 3.0f;
   camera_config.maximum_distance = 30.0f;
   camera_config.up   = Vector3{0, 1, 0};
   camera_config.fovy = 70;
   camera_config.projection = CAMERA_PERSPECTIVE;

   if (!sdk::orbit_camera_config_is_valid(camera_config)) {
      std::cerr << "ERR: CAMERA CONFIGURATION" << std::endl;
      std::free(values);
      values = nullptr;
      sdk::runtime_shutdown(runtime);
      return 1;
   }

   sdk::Orbit_Camera_State camera_state = {};
   camera_state.distance = 10.0f;
   camera_state.pitch = 30 * DEG2RAD;
   camera_state.yaw   = 0.0f;

   sdk::Cursor_Capture_State cursor_state = {};
   cursor_state.wants_cursor_captured = true;

   f32 text_x = 0.0f;
   f32 text_y = 0.0f;
   i32 font_size = 16;

   Cube_Palette_Handle palette{ PALETTE_2 };

   Cube_Index c{};
   Cube_Handle selected_handle = get_handle(c, CUBE_COUNT);

   while (!WindowShouldClose()) {
      // Time and input
      f32 dt = GetFrameTime();
      Frame_Input input{ poll_frame_input() };

      if (input.palette_changed) {
         palette = input.requested_palette;
      }

      // Cursor capture and orbit orientation
      bool rotate_from_mouse{ sdk::cursor_capture_update(input.cursor, cursor_state) };

      sdk::Orbit_Camera_Input camera_input = {};
      camera_input.mouse_delta = input.mouse_delta;
      camera_input.wheel_delta = input.mouse_wheel;
      camera_input.rotate_from_mouse = rotate_from_mouse;

      sdk::orbit_camera_apply_input(camera_config, camera_input, camera_state);

      sdk::Orbit_Camera_Derived derived_camera = {};
      sdk::orbit_camera_derive(camera_config, camera_state, derived_camera);

      // Camera-relative selection navigation
      i32 delta_x{ 0 };
      i32 delta_y{ 0 };
      i32 delta_z{ 0 };
      bool has_horizontal_intent{ false };
      Vector3 intended_direction{ 0, 0, 0 };

      switch (input.movement) {
      case MOVE_LEFT:
         intended_direction = Vector3Negate(derived_camera.right);
         has_horizontal_intent = true;
         break;
      case MOVE_RIGHT:
         intended_direction = derived_camera.right;
         has_horizontal_intent = true;
         break;
      case MOVE_FORWARD:
         intended_direction = derived_camera.forward;
         has_horizontal_intent = true;
         break;
      case MOVE_BACKWARD:
         intended_direction = Vector3Negate(derived_camera.forward);
         has_horizontal_intent = true;
         break;
      case MOVE_UP:
         delta_y = 1;
         break;
      case MOVE_DOWN:
         delta_y = -1;
         break;
      case MOVE_NONE: break;
      default: break;
      }

      if (has_horizontal_intent) {
         intended_direction.y = 0.0f;
         f32 absolute_x{ abs_f32(intended_direction.x) };
         f32 absolute_z{ abs_f32(intended_direction.z) };
         if (absolute_x >= absolute_z) {
            if (intended_direction.x >= 0.0f) {
               delta_x = 1;
            } else {
               delta_x = -1;
            }
         } else {
            if (intended_direction.z >= 0.0f) {
               delta_z = 1;
            } else {
               delta_z = -1;
            }
         }
      }

      i32 next_x{ clamp_i32(static_cast<i32>(c.x) + delta_x, 0, static_cast<i32>(CUBE_COUNT.x) - 1) };
      i32 next_y{ clamp_i32(static_cast<i32>(c.y) + delta_y, 0, static_cast<i32>(CUBE_COUNT.y) - 1) };
      i32 next_z{ clamp_i32(static_cast<i32>(c.z) + delta_z, 0, static_cast<i32>(CUBE_COUNT.z) - 1) };

      c.x = static_cast<u32>(next_x);
      c.y = static_cast<u32>(next_y);
      c.z = static_cast<u32>(next_z);

      selected_handle = get_handle(c, CUBE_COUNT);

      Vector3 selected_world_center{ get_world_vector3(c, CUBE_COUNT, CUBE_SPACING) };
      Camera3D camera = {};
      sdk::orbit_camera_build(camera_config, derived_camera, selected_world_center, camera);

      // Per-frame diagnostic animation
      text_x += dt * 20.0f;
      text_y += dt * 20.0f;

      if ((i32)text_x >= config.screen_width) text_x = 0.0f;
      if ((i32)text_y >= config.screen_height) text_y = 0.0f;

      Value selected_value{ values[selected_handle.id] };
      Sampled_Coordinate selected_sample{ get_sampled_coordinate(c, CUBE_DOMAIN) };

      // Focused visibility bounds
      i32 bound{ 3 };
      i32 selected_x{ static_cast<i32>(c.x) };
      i32 selected_y{ static_cast<i32>(c.y) };
      i32 selected_z{ static_cast<i32>(c.z) };

      i32 lx{ clamp_i32(selected_x - bound, 0, static_cast<i32>(CUBE_COUNT.x) - 1) };
      i32 ux{ clamp_i32(selected_x + bound, 0, static_cast<i32>(CUBE_COUNT.x) - 1) };
      i32 ly{ clamp_i32(selected_y - bound, 0, static_cast<i32>(CUBE_COUNT.y) - 1) };
      i32 uy{ clamp_i32(selected_y + bound, 0, static_cast<i32>(CUBE_COUNT.y) - 1) };
      i32 lz{ clamp_i32(selected_z - bound, 0, static_cast<i32>(CUBE_COUNT.z) - 1) };
      i32 uz{ clamp_i32(selected_z + bound, 0, static_cast<i32>(CUBE_COUNT.z) - 1) };

      u32 submitted_cubes{ 0 };

      // Draw the 3D scene, then the 2D diagnostic overlay
      BeginDrawing();
         ClearBackground(BLACK);

         BeginMode3D(camera);
            DrawGrid(10, 1);
            DrawLine3D(camera.target, Vector3Add(camera.target, Vector3Scale(derived_camera.forward, 10.0f)), BLUE);
            DrawLine3D(camera.target, Vector3Add(camera.target, Vector3Scale(derived_camera.right,   10.0f)), RED);
            DrawLine3D(camera.target, Vector3Add(camera.target, Vector3Scale(derived_camera.view_up, 10.0f)), GREEN);

            const Cube_Palette& styles{ palettes[palette.id] };

            i32 radius_squared{ square_i32(bound) };

            for (i32 z{ lz }; z <= uz; ++z) {
               i32 dz{ square_i32(z - selected_z) };
               for (i32 y{ ly }; y <= uy; ++y) {
                  i32 dy{ square_i32(y - selected_y) };
                  for (i32 x{ lx }; x <= ux; ++x) {
                     i32 dx{ square_i32(x - selected_x) };

                     i32 distance_squared{ dx + dy + dz };
                     if (distance_squared > radius_squared) continue;

                     ++submitted_cubes;

                     // 0 1 2 3 4
                     //     ^
                     // 0 1 2 3 4 5
                     //       ^

                     Cube_Index cube_index{
                        static_cast<u32>(x),
                        static_cast<u32>(y),
                        static_cast<u32>(z)
                     };
                     Cube_Handle cube_handle{ get_handle(cube_index, CUBE_COUNT) };
                     Value cube_value{ values[cube_handle.id] };

                     Vector3 p{ get_world_vector3(cube_index, CUBE_COUNT, CUBE_SPACING) };

                     DrawCubeV(p, CUBE_SIZE, styles.styles[cube_value].fill_color);
                     if (selected_handle.id == cube_handle.id) {
                        DrawCubeWiresV(p, CUBE_SIZE, LIME);
                     } else {
                        DrawCubeWiresV(p, CUBE_SIZE, styles.styles[cube_value].wire_color);
                     }

                  }
               }
            }

         EndMode3D();

         DrawText(TextFormat("seconds: %.2f", GetTime()), (i32)text_x, (i32)text_y, font_size, RAYWHITE);

         i32 y_offset = 5;
         i32 x_offset = 5;
         i32 fps = GetFPS();
         DrawText(TextFormat("FPS: %i", fps), x_offset, y_offset, font_size, YELLOW);
         y_offset += font_size;
         DrawText("Ya Boi", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;
         DrawText("ESC: Exit", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;
         DrawText(TextFormat("Palette: %u", palette.id), x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         DrawText("FORWARD: BLUE - RIGHT: RED - UP: GREEN", x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         i32 cubes_tested{ (ux - lx + 1) * (uy - ly + 1) * (uz - lz + 1) };
         DrawText(TextFormat("Tested Cubes: %i", cubes_tested), x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         DrawText(TextFormat("Submitted Cubes: %u", submitted_cubes), x_offset, y_offset, font_size, RAYWHITE);
         y_offset += font_size;

         DrawText(TextFormat("CUBE: handle = %u  value = %s | index (%u,%u,%u) sample (%.2f,%.2f,%.2f)", selected_handle.id, get_value_string(selected_value), c.x, c.y, c.z, selected_sample.x, selected_sample.y, selected_sample.z), x_offset, y_offset, font_size, RAYWHITE);

         y_offset += font_size;
         DrawText(TextFormat("CAMERA: target (%.2f,%.2f,%.2f) yaw (%.2f) pitch (%.2f) distance (%.2f) capture (%d)", camera.target.x, camera.target.y, camera.target.z, camera_state.yaw, camera_state.pitch, camera_state.distance, cursor_state.wants_cursor_captured), x_offset, y_offset, font_size, RAYWHITE);

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

#include "sdk/orbit_camera.h"

#include "raymath.h"

#include <math.h>

namespace sdk {

static float clamp_float(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

void orbit_camera_request_capture(Orbit_Camera_State& state, int should_capture)
{
    state.wants_cursor_captured = should_capture ? 1 : 0;
}

Orbit_Camera_Update_Result orbit_camera_update(
    const Orbit_Camera_Config& config,
    const Orbit_Camera_Input& input,
    Orbit_Camera_State& state,
    Camera3D& camera)
{
    if (config.minimum_distance > config.maximum_distance ||
        config.minimum_pitch > config.maximum_pitch) {
        return ORBIT_CAMERA_INVALID_CONFIG;
    }

    if (input.capture_toggle_pressed) {
        state.wants_cursor_captured = !state.wants_cursor_captured;
    }

    if (!input.is_window_focused) {
        if (state.is_cursor_captured) {
            EnableCursor();
            state.is_cursor_captured = 0;
        }
    } else {
        if (state.wants_cursor_captured && !state.is_cursor_captured) {
            DisableCursor();
            state.is_cursor_captured = 1;
            state.suppress_mouse_delta = 1;
        } else if (!state.wants_cursor_captured && state.is_cursor_captured) {
            EnableCursor();
            state.is_cursor_captured = 0;
        }
    }

    if (state.is_cursor_captured) {
        if (state.suppress_mouse_delta) {
            state.suppress_mouse_delta = 0;
        } else {
            state.yaw -= input.mouse_delta.x * config.radians_per_mouse_pixel;
            state.pitch -= input.mouse_delta.y * config.radians_per_mouse_pixel;
        }
    }

    state.distance -= input.wheel_delta * config.world_units_per_wheel_step;
    state.pitch = clamp_float(state.pitch, config.minimum_pitch, config.maximum_pitch);
    state.distance = clamp_float(state.distance, config.minimum_distance, config.maximum_distance);

    float horizontal_radius = cosf(state.pitch) * state.distance;
    float vertical_offset = sinf(state.pitch) * state.distance;

    camera.target = state.target;
    camera.position.x = state.target.x + sinf(state.yaw) * horizontal_radius;
    camera.position.y = state.target.y + vertical_offset;
    camera.position.z = state.target.z + cosf(state.yaw) * horizontal_radius;
    camera.up = config.up;
    camera.fovy = config.fovy;
    camera.projection = config.projection;

    state.was_window_focused = input.is_window_focused;

    return ORBIT_CAMERA_UPDATE_SUCCESS;
}

} // namespace sdk

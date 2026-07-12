#include "sdk/orbit_camera.h"

#include <math.h>
#include <string.h>

namespace sdk {

static float clamp_float(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }

    return value;
}

void cursor_capture_update(Cursor_Capture& cursor, int wants_capture, int window_focused) {
    int should_capture = 0;
    if (wants_capture && window_focused) {
        should_capture = 1;
    }

    if (should_capture && !cursor.is_captured) {
        DisableCursor();
        cursor.suppress_mouse_delta = 1u;
    }
    if (!should_capture && cursor.is_captured) {
        EnableCursor();
        cursor.suppress_mouse_delta = 1u;
    }
    if (should_capture && !cursor.was_window_focused && window_focused) {
        cursor.suppress_mouse_delta = 1u;
    }

    cursor.wants_capture = 0u;
    if (wants_capture) {
        cursor.wants_capture = 1u;
    }
    cursor.is_captured = 0u;
    if (should_capture) {
        cursor.is_captured = 1u;
    }
    cursor.was_window_focused = 0u;
    if (window_focused) {
        cursor.was_window_focused = 1u;
    }
}

void cursor_capture_shutdown(Cursor_Capture& cursor) {
    if (cursor.is_captured && IsWindowReady()) {
        EnableCursor();
    }

    memset(&cursor, 0, sizeof(cursor));
}

void orbit_camera_init(Orbit_Camera& orbit, Vector3 target, float yaw, float pitch, float distance,
                       float vertical_fov) {
    orbit = Orbit_Camera{};
    orbit.target = target;
    orbit.yaw = yaw;
    orbit.pitch = pitch;
    orbit.distance = distance;
    orbit.vertical_fov = vertical_fov;
    orbit.projection = CAMERA_PERSPECTIVE;
}

void orbit_camera_update(Orbit_Camera& orbit, const Orbit_Config& config, Vector2 mouse_delta,
                         float mouse_wheel, int rotate_from_mouse) {
    if (rotate_from_mouse) {
        orbit.yaw -= mouse_delta.x * config.mouse_sensitivity;
        orbit.pitch -= mouse_delta.y * config.mouse_sensitivity;
    }

    orbit.distance -= mouse_wheel * config.wheel_speed;
    orbit.pitch = clamp_float(orbit.pitch, config.minimum_pitch, config.maximum_pitch);
    orbit.distance = clamp_float(orbit.distance, config.minimum_distance, config.maximum_distance);
    orbit.vertical_fov = config.vertical_fov;
}

void orbit_camera_retarget(Orbit_Camera& orbit, Vector3 target) {
    orbit.target = target;
}

Camera3D orbit_camera_build(const Orbit_Camera& orbit) {
    Camera3D camera = {};
    float horizontal_distance = cosf(orbit.pitch) * orbit.distance;
    camera.target = orbit.target;
    camera.position.x = orbit.target.x + sinf(orbit.yaw) * horizontal_distance;
    camera.position.y = orbit.target.y + sinf(orbit.pitch) * orbit.distance;
    camera.position.z = orbit.target.z + cosf(orbit.yaw) * horizontal_distance;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = orbit.vertical_fov;
    camera.projection = orbit.projection;
    return camera;
}

} // namespace sdk

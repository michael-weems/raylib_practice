#include "sdk/orbit_camera.h"

#include <math.h>

namespace sdk {

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static void orbit_camera_rebuild(Orbit_Camera& orbit, float fovy)
{
    float cp = cosf(orbit.pitch);
    orbit.camera.position.x = orbit.target.x + sinf(orbit.yaw)*cp*orbit.distance;
    orbit.camera.position.y = orbit.target.y + sinf(orbit.pitch)*orbit.distance;
    orbit.camera.position.z = orbit.target.z + cosf(orbit.yaw)*cp*orbit.distance;
    orbit.camera.target = orbit.target;
    orbit.camera.up = Vector3{0.0f, 1.0f, 0.0f};
    orbit.camera.fovy = fovy;
    orbit.camera.projection = CAMERA_PERSPECTIVE;
}

void cursor_capture_update(Cursor_Capture& cursor, int wants_capture, int window_focused)
{
    int should_capture = wants_capture && window_focused;
    if (should_capture && !cursor.is_captured) {
        DisableCursor();
        cursor.suppress_delta = 1;
    } else if (!should_capture && cursor.is_captured) {
        EnableCursor();
    } else if (should_capture && !cursor.was_focused && window_focused) {
        cursor.suppress_delta = 1;
    }

    cursor.wants_capture = wants_capture;
    cursor.is_captured = should_capture;
    cursor.was_focused = window_focused;
}

void orbit_camera_init(Orbit_Camera& orbit, Vector3 target, float yaw, float pitch, float distance, float fovy)
{
    orbit.target = target;
    orbit.yaw = yaw;
    orbit.pitch = pitch;
    orbit.distance = distance;
    orbit_camera_rebuild(orbit, fovy);
}

void orbit_camera_update(Orbit_Camera& orbit, const Orbit_Config& c, Vector2 mouse_delta, float wheel, int rotate_from_mouse)
{
    if (rotate_from_mouse) {
        orbit.yaw -= mouse_delta.x*c.mouse_sensitivity;
        orbit.pitch -= mouse_delta.y*c.mouse_sensitivity;
    }

    orbit.distance -= wheel*c.wheel_speed;
    orbit.pitch = clampf(orbit.pitch, c.min_pitch, c.max_pitch);
    orbit.distance = clampf(orbit.distance, c.min_distance, c.max_distance);
    orbit_camera_rebuild(orbit, c.fovy);
}

void orbit_camera_retarget(Orbit_Camera& orbit, Vector3 target)
{
    orbit.target = target;
    orbit_camera_rebuild(orbit, orbit.camera.fovy);
}

}

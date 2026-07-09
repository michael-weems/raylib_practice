#include "sdk/runtime.h"

#include "raylib.h"

namespace sdk {

Runtime_Result runtime_init(Runtime_State& state, Runtime_Config& config)
{
    if (state.is_initialized) {
        return RUNTIME_ERROR_ALREADY_INITIALIZED;
    }

    state.used_defaults = 0;

    if (!config.title || config.title[0] == '\0') {
        config.title = "Software Cube Field";
        state.used_defaults = 1;
    }

    if (config.screen_width <= 0) {
        config.screen_width = 1920;
        state.used_defaults = 1;
    }

    if (config.screen_height <= 0) {
        config.screen_height = 1080;
        state.used_defaults = 1;
    }

    if (config.target_fps <= 0) {
        config.target_fps = 60;
        state.used_defaults = 1;
    }

    if (config.exit_key == 0) {
        config.exit_key = KEY_ESCAPE;
        state.used_defaults = 1;
    }

    InitWindow(config.screen_width, config.screen_height, config.title);

    if (!IsWindowReady()) {
        return RUNTIME_ERROR_WINDOW_INITIALIZATION;
    }

    SetTargetFPS(config.target_fps);
    SetExitKey(config.exit_key);

    state.is_initialized = 1;
    return RUNTIME_SUCCESS;
}

void runtime_shutdown(Runtime_State& state)
{
    if (state.is_initialized) {
        CloseWindow();
        state.is_initialized = 0;
    }
}

} // namespace sdk

#include "sdk/runtime.h"

#include "raylib.h"

namespace sdk {

int runtime_init(Runtime_State& runtime, const Runtime_Config& requested_config) {
    runtime = Runtime_State{};
    Runtime_Config config = requested_config;
    if (config.screen_width <= 0) {
        config.screen_width = 1920;
    }
    if (config.screen_height <= 0) {
        config.screen_height = 1080;
    }
    if (config.target_fps <= 0) {
        config.target_fps = 60;
    }
    if (!config.title || !config.title[0]) {
        config.title = "software renderer";
    }

    InitWindow(config.screen_width, config.screen_height, config.title);
    if (!IsWindowReady()) {
        return 1;
    }

    SetTargetFPS(config.target_fps);
    SetExitKey(KEY_ESCAPE);
    runtime.config = config;
    runtime.initialized = 1;
    return 0;
}

void runtime_shutdown(Runtime_State& runtime) {
    if (runtime.initialized && IsWindowReady()) {
        CloseWindow();
    }

    runtime = Runtime_State{};
}

} // namespace sdk

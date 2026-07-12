#pragma once

namespace sdk {

struct Runtime_Config {
    int screen_width;
    int screen_height;
    int target_fps;
    const char* title;
};

struct Runtime_State {
    Runtime_Config config;
    int initialized;
};

// These lifecycle utilities complement direct Raylib frame-loop calls. The
// application still owns WindowShouldClose, BeginDrawing, and EndDrawing.
int runtime_init(Runtime_State& runtime, const Runtime_Config& requested_config);
void runtime_shutdown(Runtime_State& runtime);

} // namespace sdk

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

int runtime_init(Runtime_State& runtime, const Runtime_Config& requested);
void runtime_shutdown(Runtime_State& runtime);

}

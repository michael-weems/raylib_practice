#include "sdk/runtime.h"

#include "raylib.h"

namespace sdk {

int runtime_init(Runtime_State& runtime, const Runtime_Config& requested)
{
    Runtime_Config c = requested;
    if (c.screen_width <= 0) c.screen_width = 1920;
    if (c.screen_height <= 0) c.screen_height = 1080;
    if (c.target_fps <= 0) c.target_fps = 60;
    if (!c.title || !c.title[0]) c.title = "software renderer";

    InitWindow(c.screen_width, c.screen_height, c.title);
    if (!IsWindowReady()) return 1;

    SetTargetFPS(c.target_fps);
    SetExitKey(KEY_ESCAPE);
    runtime.config = c;
    runtime.initialized = 1;
    return 0;
}

void runtime_shutdown(Runtime_State& runtime)
{
    if (runtime.initialized) CloseWindow();
    runtime.initialized = 0;
}

}

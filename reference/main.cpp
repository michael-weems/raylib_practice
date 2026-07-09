#include "allocators/arena_allocator.h"
#include "app/application.h"

#include "raylib.h"

#include <stdio.h>

int main(void)
{
    allocators::Allocator persistent_allocator = allocators::malloc_allocator();

    app::App_Config config = {};
    app::app_default_config(config);

    app::App_State app_state = {};
    app::App_Result result = app::app_init(persistent_allocator, config, app_state);
    if (result != app::APP_SUCCESS) {
        fprintf(stderr, "Application initialization failed: %d\n", (int)result);
        return (int)result;
    }

    while (!WindowShouldClose()) {
        app::app_update_and_render(app_state);
    }

    app::app_shutdown(persistent_allocator, app_state);
    return 0;
}

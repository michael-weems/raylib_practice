#include "allocators/arena_allocator.h"
#include "app/application.h"
#include "raylib.h"

int main(void)
{
    alloc::Allocator allocator = alloc::malloc_allocator();
    app::Application application = {};
    app::Application_Config config = app::application_default_config();

    if (app::application_init(application, config, allocator)) {
        app::application_shutdown(application);
        return 1;
    }

    while (!WindowShouldClose()) {
        app::application_frame(application);
    }

    app::application_shutdown(application);
    return 0;
}

#include "allocators/aligned_allocator.h"
#include "allocators/arena_allocator.h"
#include "app/application.h"

#include "raylib.h"

#include <stdio.h>

int main(void) {
    alloc::Allocator persistent_allocator = alloc::aligned_allocator_create();
    app::Application_Config config = app::application_default_config();
    app::Application_Memory_Requirements memory_requirements = {};
    app::Application_Result result =
        app::application_memory_requirements(memory_requirements, config);
    if (result != app::APPLICATION_RESULT_OK) {
        fprintf(stderr, "reference configuration failed: %s\n",
                app::application_result_label(result));
        return 1;
    }

    void* temporary_memory =
        alloc::allocator_allocate(persistent_allocator, memory_requirements.temporary.capacity,
                                  memory_requirements.temporary.alignment);
    if (temporary_memory == 0) {
        fprintf(stderr, "reference temporary-memory allocation failed\n");
        return 1;
    }

    alloc::Arena_Allocator frame_arena = {};
    if (!alloc::arena_allocator_initialize(frame_arena, temporary_memory,
                                           memory_requirements.temporary.capacity)) {
        fprintf(stderr, "reference frame arena initialization failed\n");
        alloc::allocator_release(persistent_allocator, temporary_memory,
                                 memory_requirements.temporary.capacity,
                                 memory_requirements.temporary.alignment);
        return 1;
    }
    alloc::Allocator temporary_allocator = alloc::arena_allocator_create_interface(frame_arena);

    app::Application application = {};
    result = app::application_init(application, config, persistent_allocator);
    if (result != app::APPLICATION_RESULT_OK) {
        fprintf(stderr, "reference initialization failed: %s\n",
                app::application_result_label(result));
        app::application_shutdown(application, persistent_allocator);
        alloc::allocator_release(persistent_allocator, temporary_memory,
                                 memory_requirements.temporary.capacity,
                                 memory_requirements.temporary.alignment);
        return 1;
    }

    int exit_code = 0;
    while (!WindowShouldClose()) {
        result = app::application_frame(application, temporary_allocator);
        alloc::arena_allocator_reset(frame_arena);
        if (result != app::APPLICATION_RESULT_OK) {
            fprintf(stderr, "reference frame failed: %s\n", app::application_result_label(result));
            exit_code = 1;
            break;
        }
    }

    app::application_shutdown(application, persistent_allocator);
    alloc::allocator_release(persistent_allocator, temporary_memory,
                             memory_requirements.temporary.capacity,
                             memory_requirements.temporary.alignment);
    return exit_code;
}

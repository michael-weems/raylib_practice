#ifndef REFERENCE_SDK_RUNTIME_H
#define REFERENCE_SDK_RUNTIME_H

namespace sdk {

enum Runtime_Result {
    RUNTIME_SUCCESS = 0,
    RUNTIME_ERROR_ALREADY_INITIALIZED,
    RUNTIME_ERROR_WINDOW_INITIALIZATION
};

struct Runtime_Config {
    const char* title;
    int screen_width;
    int screen_height;
    int target_fps;
    int exit_key;
};

struct Runtime_State {
    int is_initialized;
    int used_defaults;
};

Runtime_Result runtime_init(Runtime_State& state, Runtime_Config& config);
void runtime_shutdown(Runtime_State& state);

} // namespace sdk

#endif // REFERENCE_SDK_RUNTIME_H

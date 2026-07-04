#ifndef SDK_RUNTIME_H
#define SDK_RUNTIME_H

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

// Initializes Raylib and applies defaults to incomplete configuration:
// title       -> "Ya BOI"
// width       -> 1920
// height      -> 1080
// target_fps  -> 60
// exit_key    -> Escape
//
// The configuration is intentionally mutable so the caller receives the
// effective values. State remains zero if initialization fails.
Runtime_Result runtime_init(
   Runtime_State& state,
   Runtime_Config& config
);

// Closes the Raylib window only when this state owns an initialized runtime.
// Safe to call with zero-initialized state or more than once.
void runtime_shutdown(Runtime_State& state);

} // namespace sdk

#endif // SDK_RUNTIME_H

# Project Context

Read this file before every task. Keep it below 200 lines.

## Current mission

Coach a novice graphics programmer through rebuilding the software-rendered
cube-field application from scratch. The durable staged plan is
`CURRICULUM.md`. Work on one checkpoint at a time; every checkpoint must leave
an executable that launches and displays a visible result.

The complete prior implementation is preserved by Git tag `baseline`. Current
`master` is the learner's rebuild. Treat `baseline` as a rubric and recovery
reference, not as code to copy before attempting a checkpoint.

## Current progress

- Checkpoint 1 is complete and reviewed: the minimal app opens a 1920x1080
  Raylib window, clears every frame, closes cleanly, and builds with
  `GRAPHICS_API_OPENGL_SOFTWARE` plus `PLATFORM_DESKTOP_RGFW`.
- Checkpoint 2 is complete and reviewed: visible text moves using persistent
  position state and `GetFrameTime()`-scaled speed. It is committed as
  `0f6ceb6`.
- Checkpoint 3 is complete and reviewed: a fixed screen-space overlay shows
  FPS, title, and exit guidance after the animated content. It is committed as
  `f52678c`.
- Checkpoint 4 is complete and reviewed: `sdk/runtime` owns configuration,
  initialization status, failure reporting, and explicit safe shutdown while
  direct Raylib frame-loop calls remain available. The implementation commits
  culminate at `9b11d73`; an empty-title correction is currently uncommitted.
- Checkpoint 5 is complete and reviewed: a fixed perspective camera renders a
  ground grid and color-coded X/Y/Z axes beneath the 2D overlay. The learner
  can explain camera basis, field of view, and perspective versus orthographic
  projection. These changes are not yet committed.
- Checkpoint 6 is complete and reviewed (`bf72ba2`): one cube centered at the origin uses
  full dimensions `(2,2,2)`, a filled pass, and a contrasting wire pass. The
  learner understands center, half-extents, bounds, and why fill-then-wire is
  the robust overlay convention even though both orders looked identical in
  this software-rendered scene. These changes are not yet committed.
- Checkpoint 7 is complete and reviewed: target, yaw, pitch, and distance are
  authoritative orbit state; `Camera3D.position` is derived every frame. The
  camera circles at constant distance and the learner understands the pitch
  triangle and yaw split. These changes are not yet committed.
- Checkpoint 8 is complete and reviewed (`4183cfe`): mouse delta drives
  yaw/pitch directly, wheel delta drives distance, pitch stays within +/-85
  degrees, and distance stays within `[3,30]`. The learner understands
  accumulated input versus time-based rates and camera pole/zoom limits.
- Checkpoint 9 is complete and reviewed (`40e680b`): Raylib cursor capture toggles on
  right-click, focus loss releases physical capture without changing the
  desired mode, focus regain restores it, and one synthetic delta is discarded
  after capture.
- Checkpoint 10 is complete and reviewed: `sdk/orbit_camera` owns reusable
  orbit policy, cursor transitions, delta suppression, clamping, and Raylib
  `Camera3D` derivation while the app samples raw Raylib input. The integrated
  app builds and preserves the interactive cube scene. These changes are not
  yet committed.
- Checkpoint 11, application lifecycle and configuration, is next. Resume from
  `CURRENT_STEP.md` and coach only that scope.

## Coaching contract

- Teach concepts and ask diagnostic/design questions before giving solutions.
- Give goals, constraints, relevant APIs, documentation, and graduated hints.
- Do not provide exact implementation steps or finished code unless the user
  explicitly asks after making an attempt.
- Review the user's work against the current curriculum checkpoint only; avoid
  pulling later architecture forward prematurely.
- Keep each checkpoint small, buildable, runnable, and visually inspectable.
- Encourage a commit after each passing checkpoint.
- Use the `baseline` comparison only after the user's version works, or when the
  user explicitly requests it.
- Preserve the user's code and learning decisions. Diagnose before editing;
  implement only when asked.

## Final target

- C++11, Raylib 6.1-dev, RGFW, and Raylib `rlsw` CPU software rendering.
- 1920x1080 interactive 3D cube field, targeting 60 FPS and requiring 30+ FPS.
- Logical field: `500 x 1000 x 100`, cube size `1`, spacing `5`.
- Deterministic immutable values A/B/C/D; palette-defined fill and edge colors.
- Focused mode: radius-five cubes, radius-three face text, cube selection,
  camera-relative navigation including Y, compass, and local boundary grids.
- Birds-eye mode: coarse non-overlapping exterior shell, no interior/text,
  field-scale compass, picking back into a source cube.
- Transparent geometry is sorted far-to-near and rendered correctly on `rlsw`.

## Final design constraints

- Plain structs and free functions; C-style/data-oriented C++.
- No application classes, inheritance, RAII ownership, exceptions, STL
  containers, smart pointers, templates, lambdas, or iterator-style loops.
- Explicit for-loops and compact, sequential hot data.
- Required borrowed inputs use references. Pointers represent backing memory,
  C strings/callback contexts, or pointer-plus-count streams.
- Public state supports safe zero initialization and explicit shutdown.
- Expected failures use integer result codes.
- Stable array identities use one-based 32-bit handles; index zero is an
  all-zero stub. Transient command streams remain zero-based dense arrays.
- Project allocation APIs accept an `Allocator&`; the composition root chooses
  memory policy and placement.
- Frame arena is non-owning, has no fallback, and resets at frame end.
- Rendering APIs are immediate-mode and retain no application commands.
- Vendored Raylib under `vendor/raylib` is immutable.

## Intended final modules

- `src/allocators`: generic allocator, aligned persistent allocator, arena.
- `src/sdk`: Raylib-specific runtime, orbit camera, rendering primitives.
- `src/app`: field/data/palettes, views/picking, input, controller, renderer,
  application lifecycle.
- `src/main.cpp`: composition root and frame loop only.
- `tests/core_tests.cpp`: nonvisual invariants.

## Repository state and tools

- `baseline` points to the finished reference implementation and report.
- `REPORT.md` explains the finished architecture and Graphics Rendering 101.
- `CURRICULUM.md` is the authoritative rebuild sequence.
- `3D_SPACE_CURRICULUM.md` is a separate 12-week/60-session mathematics
  practice track; it does not change progress through the main curriculum.
- `CURRENT_STEP.md` is the short restart/handoff note.
- `build` configures/builds with CMake, Ninja, and MSVC.
- `make` builds and launches the executable.
- Current `CMakeLists.txt` has been reduced to `src/main.cpp`; evolve its source
  list only as curriculum modules are introduced.
- Build output is `out/build/software_renderer.exe`.

## Raylib notes

- Version: 6.1-dev (`6.0-162-g87137417`).
- Use `PLATFORM=RGFW` and `OPENGL_VERSION=Software`.
- `RLSW_USE_SIMD_INTRINSICS` is disabled because the vendored MSVC path fails
  around `_mm_loadu_si32`.
- Generic Raylib logs may say OpenGL/GPU/VRAM even when `rlsw` performs scene
  rasterization on the CPU.

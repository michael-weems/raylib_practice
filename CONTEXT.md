# Project Context

Read this file before every task. Keep it below 200 lines.

## Mission

Coach the learner through rebuilding a CPU software-rendered Raylib cube-field
application under `src/`. Work one small checkpoint at a time. Every checkpoint
must end with a buildable executable and a visible result.

The learner works best by doing:

- give a concrete runnable target, constraints, useful hints, and only the
  concepts newly required by the checkpoint;
- let the learner implement before reviewing structure;
- answer questions through graduated hints, providing finished code only when
  explicitly requested or when a concept has stopped being productive;
- review and repair the running attempt, then reflect on lessons learned;
- extract modules only after working code creates reuse, ownership, or policy
  pressure;
- encourage a commit after each passing checkpoint.

Do not turn checkpoints into up-front architecture quizzes. Preserve learner
code and decisions; diagnose before editing and implement only when asked.
Do not automate GUI input, screenshots, or visual smoke tests. Rely on the
learner's runtime observations; use static review, builds, and focused core
tests for coach-side verification.

## Authoritative training state

- `CURRICULUM.md` contains the durable 50-checkpoint route. Its progress ledger
  is the single authoritative record of completed work.
- `CURRENT_STEP.md` contains only the detailed brief for the active checkpoint.
- Checkpoints 1 through 13 are complete; Checkpoint 14 is active.
- `3D_SPACE_CURRICULUM.md` is an independent 12-week mathematics practice track
  and does not change main-curriculum progress.

Do not duplicate checkpoint history in this file.

## Reference roles

- `reference/` is the definitive finished behavioral and architectural oracle.
  It is an independent CMake project and must not link with the learner app.
- The Git tag `baseline` is older historical design evidence. It may explain a
  past decision but does not override the current reference or curriculum.
- Neither reference is starter code. Let the learner attempt a checkpoint
  before comparing implementations unless they request study or are blocked.
- `REPORT.md` explains the finished reference architecture and Graphics
  Rendering 101 while clearly distinguishing it from the in-progress `src/`.

## Final application target

- C++11, Raylib 6.1-dev, RGFW, and Raylib `rlsw` CPU software rendering.
- 1920x1080 interactive 3D cube field; target 60 FPS and require at least 30 FPS.
- Logical field `500 x 1000 x 100`, cube size `1`, spacing `5`.
- Deterministic immutable A/B/C/D values with palette-defined fill/edge colors.
- Focused mode: Euclidean radius-five cubes, radius-three face text, selection,
  camera-relative navigation including Y, compass, and bounded boundary grids.
- Birds-eye mode: coarse non-overlapping exterior shell, no interior/cube text,
  large field compass, and picking back to the exact representative source.
- Transparent faces render far-to-near after opaque geometry and behave
  correctly on `rlsw` without vendor changes.

## Durable design constraints

- Data-oriented C-style C++: plain structs, free functions, explicit loops, and
  compact sequential hot data.
- No application classes, inheritance, RAII ownership, exceptions, STL
  containers, smart pointers, templates, lambdas, or iterator-style loops.
- "Wide code" means descriptive names and clear expressions, never several
  statements packed onto one line.
- Avoid ternary operators; prefer explicit control flow.
- References represent required borrowed inputs. Pointers represent backing
  memory, C strings/callback contexts, or pointer-plus-count streams.
- Public state supports useful all-zero initialization and explicit shutdown.
- Expected failures use integer result codes.
- Stable array identities use one-based 32-bit handles. Index zero is an
  all-zero stub; transient dense command streams may remain zero-based.
- Allocation APIs eventually accept `Allocator&`; the application chooses
  memory placement. The frame arena is non-owning, has no fallback, and resets
  after each frame.
- Immediate-mode rendering may collect/sort transient commands within one
  frame, but retains no application scene or commands across frames.
- SDK utilities complement Raylib; do not create a generic Raylib wrapper.
- `vendor/raylib` is immutable; do not edit it or add change-detection work.

## Performance reasoning

Use this priority order:

1. Draw less and eliminate unnecessary work.
2. Enumerate bounded candidates rather than scanning the 50-million-cube field.
3. Hoist loop invariants.
4. Keep hot data compact and sequential.
5. Recalculate cheap derived positions/bounds instead of loading redundant
   arrays.
6. Optimize instructions, alignment, or SIMD only after measurement.

Storage is appropriate for immutable semantic values and true persistent state.
Do not assume every calculation beats every cached load: division, hashing,
trigonometry, branches, rasterization, transparency, and glyph submission have
real costs. Measure performance claims in a clean `RelWithDebInfo` build.

## Repository tools

Prefer the PowerShell-native scripts:

- `tools\build-src.ps1`
- `tools\build-reference.ps1 -RunTests`
- `tools\build-performance.ps1 -App reference -BuildType RelWithDebInfo`
- `tools\check-builds.ps1`
- `tools\verify-repo.ps1`
- `tools\scan-repo.ps1`
- `tools\changed-files.ps1`
- `tools\format.ps1 -List`
- `tools\smoke-test.ps1 -App src -NoLaunch`

The default compiler toolchain is Clang/clang-cl; pass `-Compiler MSVC` only for
an intentional comparison. `clang-format` is expected on `PATH`. Format only
project-owned C/C++; never format vendor code. Use the isolated performance
build directories for FPS claims.

Documentation commands are `npm run docs:build` and `npm run docs:serve` after
installing the locked local documentation dependencies.

## Raylib notes

- Version: 6.1-dev (`6.0-162-g87137417`).
- Build with `PLATFORM=RGFW` and `OPENGL_VERSION=Software`.
- `RLSW_USE_SIMD_INTRINSICS` remains disabled unless the vendored/compiler path
  is proven compatible.
- Generic Raylib logs can mention OpenGL, GPU, VRAM, and textures even while
  `rlsw` performs 3D scene rasterization on the CPU.

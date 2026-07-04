# Current Training Step

## Resume here

Checkpoints 1 through 10 are complete. Checkpoint 11, **Application lifecycle
and configuration**, is next and has not started.

Checkpoint 10 introduced `sdk/orbit_camera`. The app still renders the same
orbiting cube, while `main.cpp` samples raw Raylib input and the SDK owns orbit
policy, cursor transitions, clamping, and `Camera3D` derivation.

Commit Checkpoint 10 before beginning this checkpoint.

## Checkpoint 11 target

Introduce an `app` module with explicit configuration, persistent state, and
lifecycle functions. The visible orbiting-cube application must remain
unchanged, while window and camera defaults come from one application
configuration entry point.

The lifecycle vocabulary should cover:

- initialization;
- whether the application should continue running;
- per-frame update;
- rendering;
- shutdown.

## Design challenge

Before coding, classify what currently belongs to:

- composition and process ownership in `main.cpp`;
- reusable Raylib policy in `sdk`;
- application-specific configuration and state in `app`;
- one-frame input and derived render data.

Keep `main.cpp` boring: compose zero-initialized objects, configure, initialize,
loop, and shut down. Do not hide expected failures behind exceptions or RAII.

## Visible finish

The same cube, orbit controls, cursor behavior, animation, and diagnostics still
run. Application/window/camera defaults originate from one configuration path,
and forcing one invalid configuration value fails startup cleanly.

## Learning gate

Checkpoint 11 is complete when the learner can explain:

- what `main.cpp` still owns and why;
- the boundary between `app` and `sdk`;
- which state persists across frames;
- why initialization reports expected failure with a result code;
- why shutdown remains explicit even with safe zero initialization.

## Coaching notes

Start with ownership classification and header-only data/API design. Review the
header before writing implementation. Do not introduce cube-field data yet;
that begins in Checkpoint 12.

## Resume prompt

> I am starting Checkpoint 11 from `CURRENT_STEP.md`. Help me classify the
> current `main.cpp` responsibilities before I design the `app` header. Use the
> hint ladder and do not give me the finished interface.

After Checkpoint 11 passes, update the progress ledger, encourage a commit, and
move this file to Checkpoint 12.

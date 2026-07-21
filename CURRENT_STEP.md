# Current Training Step

## Resume here

Checkpoints 1 through 16 are complete. Checkpoint 17 is **Persistent selected
target**.

The highlighted coordinate in the current frame loop is temporary local data,
while the orbit camera still targets the world origin. This checkpoint makes
selection persistent and connects it to the camera's focus point.

Build directly in `main.cpp`. Do not create an application module, controller
framework, or refactor checkpoint.

## New graphics concept: moving an orbit's focus point

Your orbit camera is derived from four authoritative values:

```text
target + yaw + pitch + distance -> camera position and Camera3D
```

Changing only `target` translates the complete orbit sphere through world space.
Yaw, pitch, and distance still describe the camera's orientation and radius
relative to that new focus point.

For cube selection, keep the stable cube handle as persistent identity:

```text
selected handle -> grid coordinate -> world-space cube center -> camera target
```

The coordinate and center are derived answers. Storing all three as independent
authoritative state would allow them to disagree.

## Frame-order concept

The camera used for drawing must reflect this frame's selection:

```text
poll selection event
    -> update selected handle
    -> derive coordinate and center
    -> assign orbit target
    -> derive Camera3D
    -> draw and report the same selected cube
```

If camera derivation happens before selection changes, the scene can display one
selected cube while the camera targets the previous cube for a frame. This is a
small example of why update order matters in interactive graphics.

## Build brief

1. Move the selected cube handle outside the frame loop so it persists between
   frames.
2. Initialize it from a known valid coordinate near the field center.
3. Add one temporary discrete key, such as `N`, that toggles between that cube
   and one other known valid cube. This is only a visible retargeting test; full
   navigation begins in Checkpoint 20.
4. Each frame, derive the selected coordinate from the selected handle.
5. Derive the selected cube's exact world-space center using the same centering
   formula used to draw cube centers.
6. Assign that center to the orbit camera target before calling the orbit-camera
   update.
7. Use the persistent selected handle for highlighting and the overlay.
8. Preserve current yaw, pitch, distance, cursor capture, zoom, palette keys,
   deterministic values, and shutdown behavior.

## A useful small helper

Camera targeting and rendering now require the same coordinate-to-world-center
calculation. A single static function for that calculation is a useful
correctness boundary, not an architecture exercise:

```text
grid coordinate + dimensions + spacing -> world-space center
```

Use it for both drawing and camera targeting. This prevents a future spacing or
even/odd-centering change from moving the visible cube without moving its camera
target, picking box, or annotations.

You choose its exact name and signature. It can remain in `main.cpp`.

## Visible finish

- The camera orbits the highlighted cube's exact center.
- Pressing the temporary toggle key snaps selection and the orbit target to the
  second cube in the same frame.
- Retargeting does not reset yaw, pitch, or distance.
- The selected wireframe and overlay change to the same cube.
- Palette switching and all existing camera/cursor behavior continue to work.

## Constraints

- Store one selected handle, not a pointer to a cube.
- Do not store a per-cube position array.
- Do not directly push the camera position by the target delta; derive it from
  target/yaw/pitch/distance through the existing orbit update.
- Use `IsKeyPressed()` for the temporary discrete toggle.
- Process retargeting before camera derivation to avoid a one-frame mismatch.
- Keep rendering immediate-mode.
- Keep hot cube values contiguous and unchanged; selection adds no per-cube
  storage.
- No module extraction is required.

## Just-in-time hints

- For each axis, the field-center coordinate can begin with integer
  `dimension / 2`; either of the two middle coordinates is a valid convention
  for an even dimension.
- Choose the second coordinate by changing one axis by one while remaining in
  bounds.
- Your existing handle-to-coordinate conversion already reports whether the
  selected identity is valid.
- The same helper should replace the three repeated position expressions in the
  render loop.
- If the camera orbits between cubes rather than around the selected cube,
  inspect whether `camera_state.target` is updated before `orbit_camera_update()`.

## References if blocked

- `REPORT.md`, sections 8.1–8.2 and 10.
- `raylib.h` definitions for `Camera3D` and `Vector3`.
- `raymath.h` vector addition/subtraction helpers.
- `3D_SPACE_CURRICULUM.md` orbit-camera exercises.

Do not inspect the reference implementation before attempting this checkpoint
unless you are genuinely blocked.

## Review target

Submit the running implementation when ready. Review will check:

- selected identity persists outside the frame loop;
- coordinate and center are derived rather than duplicated state;
- drawing and camera targeting use identical center math;
- retargeting happens before camera derivation;
- yaw, pitch, distance, and capture behavior survive a snap;
- overlay, highlight, and camera agree on the selected cube;
- no redundant per-cube storage or architecture work appeared;
- the learner build passes.

Reflection follows after it works: which values moved, which remained unchanged,
and why changing the target translated rather than rotated the orbit.

## Resume prompt

> I am starting Checkpoint 17 from `CURRENT_STEP.md`: persist one selected cube
> handle, derive its world center, and make the existing orbit camera snap its
> target between two cubes without resetting orientation or zoom.

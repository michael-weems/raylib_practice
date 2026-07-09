# Current Training Step

## Resume here

Checkpoints 1 through 10 are complete. Checkpoint 11 is now **A tiny cube
field**. The premature application-lifecycle refactor has moved to Checkpoint
17, after working field data provides evidence for useful boundaries.

Build the feature directly in the current application. Do not create `app`
modules in this checkpoint.

## Build brief

Replace the single hard-coded cube with a small cube field rendered using three
explicit nested loops.

Start with these values:

- dimensions: `3 × 3 × 3`;
- cube dimensions: keep the current visible cube size initially;
- spacing: choose a center-to-center distance that leaves obvious gaps;
- field center: world origin.

For each integer grid coordinate, derive a world-space center and call the same
filled-cube and wire-cube Raylib drawing functions already used by the app.
Do not allocate or store a position per cube.

## Visible finish

The executable shows 27 evenly spaced cubes centered around the origin. Orbit,
zoom, cursor capture, grid, axes, animation, and diagnostics continue working.

Temporarily change one dimension, such as X from 3 to 5, and verify only the
expected world axis grows. Restore `3 × 3 × 3` afterward.

## Constraints

- Keep this first implementation near the current drawing code.
- Use ordinary integer loop variables and explicit `for` loops.
- Derive positions during rendering; no position array and no allocation.
- Preserve the existing SDK interfaces; no new SDK abstraction is justified by
  this feature yet.
- Keep the project runnable throughout the attempt.

## Just-in-time hints

- Solve the center coordinate for the sequence `-1, 0, +1` first.
- A cube's world center is its centered integer coordinate multiplied by the
  center-to-center spacing.
- Keep X, Y, and Z visibly distinguishable using the existing axis lines.
- If the field looks like one solid block, spacing is smaller than or equal to
  the cube width.

## References if blocked

- Current `DrawCubeV` and `DrawCubeWiresV` calls in `src/main.cpp`.
- `vendor/raylib/examples/models/models_geometric_shapes.c`.
- `REPORT.md`, sections 8.1 and 8.5, only if the centering relationship is
  unclear after trying it visually.

## Review target

Submit the running implementation when ready. Review will check cube count,
centering, axis mapping, spacing, explicit loops, absence of stored positions,
and preservation of existing controls. Reflection follows the working review.

## Resume prompt

> I am implementing the revised Checkpoint 11 from `CURRENT_STEP.md`: a direct
> `3 × 3 × 3` cube field. I will ask for hints if blocked and submit the running
> result for review.

After Checkpoint 11 passes, update the ledger and move to centering arbitrary
odd/even dimensions in Checkpoint 12 using the same build-first model.

# Current Training Step

## Resume here

Checkpoints 1 through 12 are complete. Checkpoint 13 is **One-based cube
handles**.

You already render an arbitrary even/odd cube field centered around world
origin by deriving cube centers inside the render loops. Now add a compact,
stable identity for a cube: a one-based integer handle.

Keep the work in the current application for this checkpoint. Do not create an
`app` module yet.

## Concept

A handle is an integer identity that refers to something stored or implied
elsewhere. In this project, handle `0` means "no cube" or "invalid cube."
Every real cube starts at handle `1`.

This lets future code pass around cube identity without passing pointers,
without needing `nullptr` checks, and without storing world positions. Later,
when we add a value array, index `0` can be a harmless all-zero stub and real
cube values can live at indexes `1..cube_count`.

For now, the cube positions are still implicit. The handle exists only as a
reversible identity for `(x, y, z)`.

## Build brief

Add helper logic that can:

- convert a valid `(x, y, z)` grid coordinate into a one-based cube handle;
- convert a valid one-based cube handle back into `(x, y, z)`;
- treat handle `0` as invalid / no cube;
- reject out-of-bounds coordinates without producing a real handle.

Then choose one highlighted cube coordinate, convert it to a handle, convert the
handle back to coordinates, and draw that cube with a distinct fill/wire color.

Show the highlighted cube's handle and recovered coordinate in the 2D overlay.

## Visible finish

The executable shows the same centered cube field, but one cube is visibly
highlighted.

The overlay reports something like:

```text
selected handle: 137
selected coord:  1, 3, 4
round trip:      ok
```

Changing the chosen highlighted coordinate should move the highlight to the
expected cube and still report a valid round trip.

## Constraints

- Use a 32-bit unsigned integer type for cube handles.
- Reserve handle `0`; real handles begin at `1`.
- Use explicit arithmetic and explicit `for` loops.
- Do not allocate.
- Do not store per-cube positions.
- Do not introduce an `app` module yet.
- Keep the current SDK interfaces unchanged.
- Keep the project runnable throughout the attempt.
- Avoid ternary operators.

## Just-in-time hints

- First solve the zero-based flattening formula for `(x, y, z)`.
- Decide which axis is contiguous in memory. The current render loop has `x` as
  the innermost loop, so making X contiguous will match the hot-loop direction.
- A common zero-based formula shape is:

  ```cpp
  flat = x + (y * width_x) + (z * width_x * width_y);
  ```

- The one-based handle rule is only the final identity wrapper:

  ```cpp
  handle = flat + 1;
  flat = handle - 1;
  ```

- To recover coordinates from `flat`, peel the dimensions back off with
  division and remainder.
- Test the first cube, last cube, and handle zero mentally before trusting the
  overlay.

## References if blocked

- `REPORT.md`, section 8.2 for one-based handles and zero stubs.
- `src/main.cpp` render loops, especially the current `x/y/z` order.
- `CURRICULUM.md`, Checkpoint 13.

## Review target

Submit the running implementation when ready. Review will check:

- handle `0` is never treated as a real cube;
- first and last valid coordinates round-trip correctly;
- out-of-bounds coordinates fail safely;
- the highlighted cube uses the recovered coordinate, not the original literal;
- existing orbit, zoom, cursor capture, grid, axes, field centering, and overlay
  still work.

Reflection follows after the executable works.

## Resume prompt

> I am implementing Checkpoint 13 from `CURRENT_STEP.md`: one-based cube
> handles with a visible highlighted cube and overlay round-trip diagnostics.
> I will ask for hints if blocked and submit the running result for review.

After Checkpoint 13 passes, update the ledger and move to Checkpoint 14: cube
values A/B/C/D.

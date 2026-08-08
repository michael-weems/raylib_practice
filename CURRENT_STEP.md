# Current Training Step

## Resume here

Checkpoints 1 through 17 are complete. Checkpoint 18 is **A bounded focused
region**.

The application currently submits every cube in the field every frame. This
checkpoint changes focused rendering so it directly enumerates only a small
coordinate box around the selected cube.

Build directly in `main.cpp`. Keep the existing camera, palette, value, and
selection behavior intact. Do not create a visibility system or new module.

## New graphics concept: bounded candidate generation

Large scenes are rarely rendered by blindly submitting every possible object.
The renderer first constructs a cheap set of plausible candidates, then later
applies more precise rejection tests.

For this checkpoint, the candidate region is an axis-aligned box in **grid
space**, not world space:

```text
selected grid coordinate + radius
    -> clamp against field dimensions
    -> directly enumerate the remaining coordinate box
    -> derive handles and world centers
    -> submit cubes immediately
```

The box radius is five cubes on every axis. Do not apply spherical distance
culling yet; the eight box corners should remain visible. Checkpoint 19 adds
that narrower test.

## Why direct enumeration matters

There are two possible approaches:

```text
scan the whole field -> ask whether each cube is nearby

calculate nearby bounds -> visit only cubes that can be nearby
```

The second approach scales with the visible neighborhood instead of total field
size. A centered radius-five box contains at most `11 * 11 * 11 = 1331`
candidates regardless of whether the complete field contains thousands or
millions of cubes.

This is the first major software-rendering performance lesson in the project:
the fastest cube is the cube whose geometry you never submit.

## Build brief

1. Make the temporary field large enough that a radius-five box does not cover
   the entire field. Preserve the arbitrary min/max/step configuration and
   centered world-space behavior.
2. Keep the focused radius as a single integer value outside the hot loops.
3. After deriving the selected `Cube_Index`, calculate the minimum and maximum
   candidate coordinate for X, Y, and Z.
4. Clamp each axis independently to the valid zero-based index range for that
   dimension.
5. Replace the full-field render traversal with loops over only those bounded
   coordinates. Keep X as the innermost, contiguous loop.
6. Continue deriving handles and world positions rather than storing a visible
   cube array.
7. Count how many candidates the bounded loops visit and show that count in the
   existing 2D overlay.
8. Adjust the temporary selection test so you can visibly compare a target near
   the field center with a target on an edge or corner.

## The signed/unsigned boundary trap

`Cube_Index` uses unsigned integers, but subtracting a radius near coordinate
zero is conceptually signed arithmetic:

```text
selected x = 2
radius     = 5
raw min x  = -3
clamped x  = 0
```

Subtracting directly in an unsigned type wraps to a very large positive value.
Choose a calculation order or temporary type that can represent the negative
intermediate, clamp it, and only then use the result as an array coordinate.

The upper side has a related distinction:

```text
dimension count = N
valid coordinates = 0 through N - 1
```

Decide whether your loop bounds are inclusive or half-open and keep that choice
consistent. Both conventions work; mixing them loses or adds a layer.

## Cache and hot-loop intent

The existing flattening order makes X contiguous:

```text
handle = x + y*width_x + z*width_x*width_y
```

Keeping X innermost means successive iterations read successive `Value` bytes.
That works naturally with cache-line fetches and hardware prefetching. Calculate
the six region bounds once before drawing rather than clamping or recomputing
them for every candidate.

Do not add a persistent list of visible handles. Immediate-mode rendering may
derive and submit this frame's bounded candidates directly.

## Visible finish

- With selection near the center, an `11 x 11 x 11` cube box follows the camera
  target when the field is large enough in every dimension.
- With selection at a boundary, the neighborhood is clipped cleanly instead of
  wrapping around or accessing invalid values.
- Box corners remain visible; this is intentionally not a sphere yet.
- The selected cube remains highlighted and centered by the orbit camera.
- The overlay reports the number of candidates submitted.
- Camera orbit, zoom, cursor capture, palette switching, and shutdown still
  behave as before.

## Self-checks

- At a sufficiently interior coordinate, predict `1331` submitted cubes.
- At a field corner with at least six cubes along every axis, predict
  `6 * 6 * 6 = 216` submitted cubes.
- Try all six boundaries, not only coordinate zero.
- Confirm that changing semantic min/max values without changing the step count
  does not shift the rendered field away from world origin.
- Confirm that the last valid handle remains below `CUBE_TOTAL_COUNT`.

## Constraints

- Enumerate the bounded region directly; do not scan the complete field and
  reject distant cubes.
- Do not allocate during the frame loop.
- Do not store per-cube positions or a retained visible-object list.
- Keep X as the innermost traversal dimension.
- Calculate bounds once per frame after selection is known.
- Keep rendering immediate-mode.
- Do not add Euclidean distance or `sqrt()` yet.
- No module extraction or general-purpose culling API is required.

## Hints if blocked

- Treat the problem as three independent one-dimensional intervals before
  thinking about the nested loops.
- Raylib does not need to know about these bounds; this is application-side
  candidate generation before calling `DrawCubeV()`.
- The selected coordinate already exists before camera derivation and drawing;
  it is the center of the candidate region.
- A center coordinate can be derived from each dimension count using integer
  division. A corner can use coordinate zero or the final valid coordinate.
- Report the candidate count before attempting any performance conclusions.

## References if blocked

- `REPORT.md`, sections 12.2 and 13.
- `raylib.h` for `DrawCubeV()` and `DrawCubeWiresV()`.
- `CURRICULUM.md`, Checkpoints 18 and 19, for the box-versus-sphere progression.

## Review target

Submit the running implementation when ready. Review will check:

- all six bounds clamp correctly;
- no unsigned underflow occurs near coordinate zero;
- traversal visits only the bounded candidate box;
- X remains contiguous;
- candidate counts match center and corner predictions;
- handles and value reads remain in range;
- selection, camera targeting, highlighting, and overlay agree;
- no per-frame allocation or retained render list appeared;
- the learner build passes.

Reflection follows after it works: why does calculating a small candidate region
scale with visible work, and why are bounds best calculated outside the hot
loops?

## Resume prompt

> I am starting Checkpoint 18 from `CURRENT_STEP.md`: calculate a radius-five
> grid-space box around the selected cube, clamp it to the field, and directly
> render only those bounded candidates while reporting the submitted count.

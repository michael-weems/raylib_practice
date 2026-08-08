# Current Training Step

## Resume here

Checkpoints 1 through 18 are complete. Checkpoint 19 is **Euclidean radius
culling**.

Checkpoint 18 reduced the complete field to a small axis-aligned candidate box.
This checkpoint keeps that box as a broad phase, but rejects its corner cubes
before doing cube-specific memory access, coordinate conversion, or drawing.

Build directly in `main.cpp`. Preserve the current radius of three, bounded
loops, camera, selection, palettes, and overlay. No module extraction is needed.

## New graphics concept: broad phase and narrow phase

Visibility and collision systems often use multiple tests ordered from cheapest
and broadest to more precise and expensive:

```text
complete field
    -> clamped coordinate box       broad phase
    -> squared-distance test        narrow phase
    -> handle/value/world position  accepted cube work
    -> Raylib draw submission       software rasterization
```

The box guarantees a small upper bound on work. The sphere test then removes
the box corners. This is more efficient than applying the distance test to the
entire field, and much cheaper than asking the software renderer to process
geometry that cannot belong to the focused neighborhood.

This checkpoint classifies cube **centers** in grid space. It does not test
whether the physical volume of a cube intersects a mathematical sphere.

## Grid-space Euclidean distance

For one candidate coordinate and the selected coordinate:

```text
dx = candidate x - selected x
dy = candidate y - selected y
dz = candidate z - selected z

distance squared = dx*dx + dy*dy + dz*dz
radius squared   = radius*radius
```

Accept the candidate when its squared distance is less than or equal to the
squared radius. The equality includes cubes whose centers lie exactly on the
sphere boundary.

Because spacing is currently uniform on X, Y, and Z, a sphere in grid space is
also a uniformly scaled sphere in world space. Different spacing per axis would
turn this grid-space sphere into an ellipsoid in world space.

## Why avoid `sqrt()`?

The ordinary distance formula ends with a square root:

```text
distance = sqrt(dx*dx + dy*dy + dz*dz)
```

Square root is unnecessary when you only need to compare distances. For
nonnegative values, squaring preserves order:

```text
distance <= radius

has the same classification as

distance_squared <= radius_squared
```

The squared form uses integer subtraction, multiplication, addition, and one
comparison. It is exact for these small grid offsets and avoids floating-point
conversion and square root in the candidate loop.

## Build brief

1. Keep the radius-three clamped box from Checkpoint 18 unchanged.
2. Preserve the existing box-candidate count; it reports broad-phase work.
3. Add a separate submitted-cube count and reset it once per frame.
4. Inside the bounded loops, calculate the signed X/Y/Z offset from the
   selected coordinate.
5. Compare squared distance with squared radius and skip candidates outside the
   sphere.
6. Perform the rejection before deriving a handle, loading `values[]`, deriving
   a world position, or calling a Raylib draw function.
7. Increment the submitted count only for accepted cubes.
8. Show both tested candidates and submitted cubes in the 2D overlay.

## Signed arithmetic again

The candidate and selected coordinates are unsigned identities, but their
difference can be negative:

```text
candidate x = 2
selected x  = 5
dx          = -3
```

Convert to a signed type before subtraction. Subtracting first in an unsigned
type would wrap, and squaring that wrapped value would not recover the intended
distance.

## Hot-loop reasoning

The Z offset is unchanged for an entire Z slice. The Y offset is unchanged for
an entire X row. Only the X offset changes on every innermost iteration.

After you have a correct version, look at whether your calculation naturally
allows invariant work to live at the loop level where it changes:

```text
Z loop: calculate dz and dz squared
    Y loop: calculate dy and partial squared distance
        X loop: add dx squared and classify
```

This is loop-invariant hoisting. It reduces repeated arithmetic without adding
storage or abstraction. Correctness comes first; make the direct version work
before considering this arrangement.

The distance rejection should also occur before `values[cube_handle.id]`.
Rejected cubes then cause no semantic-data load and no unnecessary world-space
math. Accepted X coordinates still access compact, mostly sequential values.

## Expected results for radius three

At a sufficiently interior selected coordinate:

```text
box candidates tested = 7 * 7 * 7 = 343
sphere cubes submitted = 123
```

At a field corner:

```text
box candidates tested = 4 * 4 * 4 = 64
sphere cubes submitted = 29
```

The corner count is a clipped octant of the discrete lattice sphere, including
the shared boundary planes and the selected center.

## Visible finish

- The focused neighborhood changes from a box to a rounded lattice cluster.
- The selected cube remains at the cluster center when away from boundaries.
- Near a field boundary, the cluster clips cleanly without wrapping.
- The overlay separately reports candidates tested and cubes submitted.
- Interior selection reports 343 tested and 123 submitted.
- Corner selection reports 64 tested and 29 submitted.
- Camera orbit, zoom, cursor capture, highlighting, and palettes still work.

## Constraints

- Keep the bounded box; do not scan the full field.
- Use squared grid distance; do not call `sqrt`, `Vector3Distance`, or a Raylib
  collision function.
- Use signed offsets before squaring.
- Reject before handle lookup, value access, world-position calculation, and
  draw submission.
- Do not allocate or construct a retained visible-cube list.
- Keep X as the innermost loop.
- Keep rendering immediate-mode.
- No SIMD or generalized culling API is required.

## Self-checks

- Temporarily inspect candidates at squared distances `0`, `1`, `8`, `9`, and
  `10`; radius three should accept through `9` and reject `10`.
- Verify the selected cube is always accepted because its squared distance is
  zero.
- Compare interior and corner counts with the expected values above.
- Try an upper boundary as well as handle zero's lower corner.
- Confirm the tested count does not change after adding the sphere test; only
  the submitted count should fall.

## References if blocked

- `REPORT.md`, sections 12.2 and 13.
- `raylib.h` for the draw calls whose submissions are being avoided.
- `3D_SPACE_CURRICULUM.md` for vector length and distance exercises.

## Review target

Submit the running implementation when ready. Review will check:

- the clamped broad-phase box remains intact;
- signed offsets are calculated correctly;
- squared-distance classification includes the radius boundary;
- rejection happens before cube-specific work;
- interior and corner tested/submitted counts match predictions;
- no `sqrt`, full-field scan, allocation, or retained render list appeared;
- selection, camera target, highlight, and overlay still agree;
- the learner build passes.

Reflection follows after it works: why is a cheap broad phase still useful when
you already have a more accurate sphere test, and why can squared distances be
compared without computing actual distances?

## Resume prompt

> I am starting Checkpoint 19 from `CURRENT_STEP.md`: keep the clamped
> radius-three candidate box, reject coordinates whose squared grid distance
> exceeds nine, and report both candidates tested and cubes submitted.

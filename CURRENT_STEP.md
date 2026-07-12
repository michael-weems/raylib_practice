# Current Training Step

## Resume here

Checkpoints 1 through 13 are complete. Checkpoint 14 is **Cube values A through
D**.

You already have an implicit cube field: coordinates derive world positions,
and one-based handles provide stable identities. This checkpoint adds the first
piece of information that genuinely differs per cube—one semantic category
byte—while leaving position and color derived.

Build this directly in the current application. Do not create an `app` module
or allocator abstraction yet.

## New concept: semantic data in a contiguous buffer

A cube value answers **what category is this cube?** It does not answer how the
cube should look. A later palette checkpoint will translate A/B/C/D into fill
and edge colors. Keeping those concerns separate lets the same immutable field
be visualized with many palettes.

The value storage should mirror the handle rule:

```text
handle:   0  1  2  3  ...  cube_count
value:    0  A  D  B  ...  C
           ^
           harmless all-zero stub
```

This is a single contiguous byte buffer with `cube_count + 1` entries. Slot
zero is reserved; real handles index their values directly. Since X is already
the innermost field-loop axis, consecutive X coordinates produce consecutive
handles and therefore consecutive value loads.

At this scale the important lesson is the layout and identity relationship,
not the allocation policy. Use the simplest application-owned storage that
fits the current field and initialize it once before entering the frame loop.

## Build brief

Add a cube-value type with five states:

- zero/none at numeric value zero;
- A, B, C, and D as the real categories;
- a one-byte underlying representation.

Then:

1. Determine the number of real cubes from the current field dimensions.
2. Create one contiguous value buffer with room for the zero stub and every
   real handle.
3. Leave slot zero in its zero-initialized state.
4. Populate slots `1..cube_count` once at startup using a simple provisional
   repeating pattern. Variation is the goal; deterministic hashing comes in
   Checkpoint 15.
5. In the existing cube render loop, derive the cube handle and load its value
   from the buffer.
6. Translate that value to a temporary visible fill color. Keep this translation
   deliberately small because Checkpoint 16 will replace it with palettes.
7. Preserve the selected-cube indication—using a distinct edge/wire color is a
   clean way to show selection without hiding the value fill.
8. Add the selected cube's A/B/C/D label to the existing overlay.
9. Release provisional storage explicitly at shutdown if the storage mechanism
   you chose requires it.

## Visible finish

The same centered field and camera controls still work, but cubes now show four
clearly distinguishable fill colors. The selected cube remains identifiable,
and the overlay reports its handle, recovered coordinate, round-trip result,
and semantic value.

Changing the provisional pattern should redistribute colors without changing
cube positions or handles.

## Constraints

- Store one byte per cube value.
- Allocate capacity for `cube_count + 1`; real data starts at index one.
- Slot zero remains the all-zero value stub.
- Initialize values once before the frame loop; do not mutate them each frame.
- Keep positions implicit and continue deriving them in the render loop.
- Do not store per-cube colors, centers, bounds, or handles.
- Do not add palette structs, deterministic hashing, an allocator interface, or
  an `app` module yet.
- Use explicit loops and control flow; avoid ternary operators.
- Preserve existing orbit, zoom, cursor, grid, axes, and overlay behavior.
- Keep the executable buildable and runnable throughout the attempt.

## Just-in-time hints

- Give the enum an explicitly byte-sized underlying type; verify the size if
  you are unsure what the compiler selected.
- Your handle already contains exactly the index needed by the value buffer.
  Conversion to coordinates is not required for the lookup.
- Populate real slots with a loop over handles or while traversing coordinates.
  Ask which version makes the one-based invariant easiest to inspect.
- A simple modulo pattern is acceptable here. Be careful that zero is reserved
  while the four real categories form a four-value cycle.
- A small helper that returns the zero stub for an invalid handle can keep the
  render loop readable without turning a pointer into cube identity.
- If the selected cube fill must show its category, emphasize selection with
  its wireframe rather than replacing the fill.

## References if blocked

- `REPORT.md`, sections 8.2, 8.3, and 8.5.
- `CURRICULUM.md`, Checkpoint 14.
- The existing handle calculation and X-innermost loops in `src/main.cpp`.
- C++ fixed-width integer types in `<cstdint>`.

Do not inspect the finished `reference/` implementation before attempting the
checkpoint unless you are genuinely blocked.

## Review target

Submit the running implementation when ready. Review will check:

- slot zero is a real all-zero stub and never populated as a cube;
- the buffer has exactly one byte-wide category per real handle;
- all accesses are bounded and use handles as identity;
- generation happens once rather than inside the frame loop;
- no redundant per-cube positions, colors, or handles were introduced;
- all four values are visibly represented;
- the selected overlay reports the same value used to draw that cube;
- existing behavior still works and the learner build passes.

Reflection follows after the executable works.

## Resume prompt

> I am starting Checkpoint 14 from `CURRENT_STEP.md`: a contiguous byte-wide
> A/B/C/D value buffer indexed by one-based cube handles. I will implement the
> runnable result first, ask for hints if blocked, and submit it for review.

After Checkpoint 14 passes, update the authoritative ledger and move to
Checkpoint 15: deterministic value generation.

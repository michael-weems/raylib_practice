# Current Training Step

## Resume here

Checkpoints 1 through 14 are complete. Checkpoint 15 is **Deterministic value
generation**.

You now have one immutable byte per cube and a direct relationship between a
one-based handle and its semantic A/B/C/D value. The provisional repeating
pattern proved that layout. This checkpoint changes only how those startup
values are generated.

Build this directly in the current application. Do not add palettes, allocator
interfaces, random-number libraries, or new modules yet.

## New concept: coordinate hashing

A deterministic hash converts a coordinate into a well-mixed integer:

```text
(x, y, z) -> coordinate mixing -> avalanche -> uint32 result -> A/B/C/D
```

It must satisfy two properties:

1. The same coordinate always produces the same result, including after an
   application restart.
2. Nearby or regularly spaced coordinates should not produce an obvious visual
   stripe, plane, or repeating block.

Your current handle-modulo pattern is deterministic, but it is strongly related
to storage order. X changes fastest, handles increase sequentially, and modulo
four repeats along that same sequence. A hash should deliberately destroy that
visible relationship while keeping generation reproducible.

An **avalanche** means that changing even one input bit affects many output
bits. Integer overflow in unsigned 32-bit arithmetic is useful here and is
defined behavior: multiplication and addition wrap modulo `2^32`.

## Build brief

Replace the provisional handle-based pattern with coordinate-derived startup
generation:

1. Keep the existing contiguous `cube_count + 1` value buffer and zero stub.
2. Add a small function that accepts a cube coordinate and returns a 32-bit
   mixed value.
3. Give X, Y, and Z distinct influence before applying an avalanche sequence.
4. During startup, traverse the field coordinates in the same X-fastest order
   used by rendering.
5. Derive each coordinate's one-based handle and store its generated value at
   that handle.
6. Map the mixed result only onto A, B, C, or D. Never generate `VALUE_NONE` for
   a real cube.
7. Leave the buffer immutable after startup.
8. Preserve cube coloring, selected wireframe, selected-value label, camera,
   axes, grid, and cleanup behavior.

The mapping from a well-mixed integer to four categories can use a small range
of result bits because four equally sized outcomes divide the integer range
exactly. The hard part is making those bits well mixed first.

## Visible finish

The cube field looks irregularly mixed rather than striped. Closing and
restarting the executable produces exactly the same arrangement.

Temporarily increasing the field dimensions is acceptable if you need a larger
sample to expose weak patterns, but keep the application responsive and retain
the arbitrary even/odd centering behavior.

## Constraints

- Generation happens once before the frame loop.
- Values depend only on stable cube coordinates and fixed program constants.
- Do not use time, frame count, `rand`, operating-system entropy, or mutable
  random-generator state.
- Slot zero remains `VALUE_NONE`.
- Every real handle receives exactly one A/B/C/D value.
- Continue storing only one semantic byte per cube.
- Do not store per-cube coordinates, handles, positions, or colors.
- Do not generate or hash values during rendering.
- Use unsigned integer arithmetic for intentional wraparound.
- Avoid ternary operators and preserve explicit control flow.
- Keep the application buildable and runnable throughout the attempt.

## Just-in-time hints

- A useful shape is one coordinate-mixing function returning `std::uint32_t`.
  Keep it independent of Raylib and rendering.
- Mixing X, Y, and Z with different fixed odd values prevents the axes from
  contributing identically.
- Avalanche steps commonly alternate XOR-with-shift and multiplication. Inspect
  the resulting behavior rather than assuming any collection of operations is
  good enough.
- With four real categories, think about how two well-mixed output bits cover
  four outcomes without generating zero.
- Use your existing coordinate-to-handle function when writing the buffer. That
  keeps storage identity in one place.
- If the result still forms stripes, test coordinates separated by powers of
  two; weak low bits often reveal themselves there.

Ask for the next level of hint if you become blocked. Start with the behavior
and inspect the pattern before pursuing a sophisticated hash.

## References if blocked

- `REPORT.md`, section 8.3.
- `CURRICULUM.md`, Checkpoint 15.
- Unsigned integer arithmetic in the Microsoft C++ language reference.
- Your existing coordinate/handle conversion in `src/main.cpp`.

Do not inspect the finished `reference/` implementation before attempting the
checkpoint unless you are genuinely blocked.

## Review target

Submit the running implementation when ready. Review will check:

- slot zero and the one-byte buffer remain correct;
- every real handle is initialized exactly once at startup;
- generation is a pure function of coordinate and fixed constants;
- real cubes receive only A through D;
- the arrangement repeats across runs;
- obvious handle-order stripes are removed;
- generation performs no per-frame work or redundant storage;
- existing behavior and cleanup remain intact;
- the learner build passes.

Reflection follows after the executable works: where weak patterns appeared,
which result bits were used, and what the avalanche changed.

## Resume prompt

> I am starting Checkpoint 15 from `CURRENT_STEP.md`: replace the provisional
> handle pattern with deterministic coordinate hashing performed once at
> startup. I will submit the running result for review.

After Checkpoint 15 passes, update the authoritative ledger and move to
Checkpoint 16: palette-defined fill and edge colors.

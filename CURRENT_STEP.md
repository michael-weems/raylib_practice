# Current Training Step

## Resume here

Checkpoints 1 through 16 are complete. Checkpoint 17 is **Refactor proven
application boundaries**. It is deliberately divided into independently
runnable substeps:

1. **17A:** extract pure cube-field rules;
2. **17B:** compose application state and frame behavior around the proven code;
3. **17C:** add focused nonvisual core tests and finish the small composition
   root.

Only substep 17A is active. Do not attempt the later substeps yet.

## Why refactor now

`main.cpp` currently proves several useful behaviors, but it also knows every
detail of them. The field's identity math, value generation, position math, and
palette rules are now stable enough to have a name and an interface.

This refactor should move working code, not reinvent it. The executable must
remain visually and interactively unchanged after every small move.

## New concept: dependency boundaries

A useful module groups code by what it knows:

```text
app/cube_field
    knows coordinates, dimensions, handles, values, positions, and palettes
    does not know windows, input keys, cameras, frame timing, or drawing order

main.cpp
    owns startup/shutdown, memory, input polling, and the frame loop
    asks cube_field for application-specific facts

sdk
    retains reusable Raylib policy already proven by runtime and orbit camera
```

The dependency direction matters. `main.cpp` may use `app/cube_field`; the
cube-field module must not call back into `main.cpp` or depend on the SDK.

A **pure** function produces its answer only from its arguments and does not
read input devices, open windows, draw, allocate hidden memory, or mutate
unrelated global state. Pure field functions will later be easy to test without
launching Raylib.

## Build target for 17A

Create `src/app/cube_field.h` and `src/app/cube_field.cpp`. Move the already
proven cube-field responsibilities out of `main.cpp`:

- coordinate and dimension data;
- one-based cube handles and zero-handle behavior;
- A/B/C/D values and labels;
- coordinate/handle conversion;
- deterministic coordinate hashing and caller-provided value generation;
- coordinate-to-world-center calculation, including even/odd centering;
- visual styles, palette data, palette handles, named palette constants, and
  palette initialization/lookup.

Use an `app` namespace so generic concepts such as a handle or coordinate do
not leak into the global namespace.

Keep these responsibilities in `main.cpp` for now:

- the executable entry point;
- allocation and release of the value buffer;
- runtime and camera configuration/state;
- Raylib input polling;
- `BeginDrawing`, `BeginMode3D`, all draw calls, and the overlay;
- selection's temporary hard-coded coordinate;
- top-level initialization, failure cleanup, and shutdown order.

This is not yet the final architecture. It is the smallest extraction justified
by the code that already works.

## Suggested hands-on sequence

Keep the application runnable between each move:

1. Create the header and implementation, add them to the existing CMake
   executable, and confirm an otherwise-empty module builds.
2. Move the field types and coordinate/handle functions. Update call sites and
   build.
3. Move value labels, hashing, and generation behind a function that fills the
   caller-owned `Value*` buffer. Allocation remains in `main.cpp`. Build and run.
4. Move the even/odd world-center calculation into a field function so the
   render loop no longer owns that formula. Build and run.
5. Move palette types, named handles, initialization, and style lookup. Build
   and run again.
6. Remove the old duplicate definitions only after every call site uses the
   module.

If a move breaks behavior, undo only that move mentally or with a small edit;
do not continue stacking extra changes on top of it.

## Interface guidance

- Put public types, constants, and function declarations in the header.
- Put function bodies and private hashing helpers in the `.cpp` file.
- Keep the existing compact value stream: one zero byte followed by one byte
  per real cube.
- Caller-provided memory is represented by a pointer because it is backing
  storage, not object identity.
- Tiny coordinates, dimensions, handles, and values may be passed by value.
  Use references for required larger borrowed records and returned palette/style
  views.
- Palette/style lookup should preserve the useful zero stubs.
- Do not allocate inside the cube-field module in this substep.
- Do not store per-cube coordinates, centers, colors, or handles.
- Do not expose private hash helpers merely because they live in another file.
- Prefer a few semantically complete functions over chains of tiny wrappers.

You decide the exact function names and grouping. Let the current call sites
tell you what the interface needs instead of designing for hypothetical users.

## Visible finish

The executable behaves exactly as it did at the end of Checkpoint 16:

- the centered field renders with deterministic values;
- palettes 1/2/3 switch fill and edge colors;
- the selected cube remains highlighted;
- orbit, zoom, capture, axes, overlay, and shutdown still work;
- `main.cpp` no longer contains cube identity, generation, position, or palette
  implementation details.

## Constraints

- Do not copy from `reference/` before attempting the extraction.
- Do not add an allocator abstraction, arena, application framework, renderer,
  input snapshot, or tests yet.
- Do not move direct Raylib frame or drawing calls into the field module.
- Do not create classes, constructors, destructors, templates, STL containers,
  or hidden ownership.
- Avoid ternary operators and iterator-style loops.
- Preserve one-based handles, all-zero stubs, data layout, traversal order, and
  immediate-mode rendering.
- Update only project-owned CMake/source files; vendor Raylib remains immutable.
- End every sequence item with a buildable, runnable application.

## If you become blocked

Classify a line by asking: "Could this produce a useful answer in a command-line
test without opening a window?" If yes and the answer is about cube-field data,
it probably belongs in `app/cube_field`. If it polls Raylib state, draws, owns
the process loop, or reports frame diagnostics, leave it in `main.cpp`.

Ask for a hint about one boundary or one function signature at a time. Do not
inspect the finished reference unless the extraction has stopped being a useful
learning exercise.

## Review target for 17A

Submit the running extraction when ready. Review will check:

- the learner build passes with the new CMake sources;
- executable behavior is unchanged according to your runtime check;
- `app/cube_field` owns cohesive pure field/value/palette rules;
- `main.cpp` still clearly owns memory and the immediate-mode frame loop;
- generation writes caller-provided storage once at startup;
- position and style data remain calculated/looked up rather than duplicated;
- public names and ownership are understandable without deep call chains;
- no premature application, allocator, rendering, or test framework appeared.

After 17A passes, reflection will identify what the extraction clarified before
substep 17B begins.

## Resume prompt

> I am starting Checkpoint 17A from `CURRENT_STEP.md`: extract the proven pure
> cube-field, value, position, and palette rules into `src/app/cube_field` while
> keeping memory ownership and immediate-mode drawing in `main.cpp`.

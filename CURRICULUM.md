# Software-Rendered Cube Field: Rebuild Curriculum

## Purpose

This curriculum guides you through rebuilding the application preserved at Git
tag `baseline`. It is a training route, not a transcription route. You will
learn the project by making one small, visible capability work at a time.

There are **50 checkpoints** in eight phases. Every checkpoint ends with an
executable you can launch and inspect. Most checkpoints add a visible behavior;
architecture-only checkpoints preserve the existing scene and add a small
diagnostic so you can prove the refactor still works.

The destination is the same as the reference application, but your intermediate
code is allowed to be simpler. Do not design the final abstraction before you
have felt the problem it solves.

## How we will work together

Every checkpoint uses a build-first, just-in-time loop:

1. **Build brief:** I give you the concrete runnable outcome, constraints,
   useful hints, and the smallest relevant references.
2. **Hands-on attempt:** You begin coding immediately. Exploration and an
   imperfect first structure are expected.
3. **Just-in-time help:** Ask questions when you encounter them. I will help
   without turning the exercise into a finished-code transcription.
4. **Run and observe:** Build, launch, and report what actually happens.
5. **Review and repair:** I review the implementation against the checkpoint;
   you make focused corrections and run it again.
6. **Reflect afterward:** Once it works, I ask a few questions that connect the
   code you just wrote to the underlying graphics or architecture concepts.
7. **Commit:** Preserve the working checkpoint before moving forward.

There is no up-front design gate. We introduce architecture when the code
creates a reason for it, then reflect on tradeoffs with concrete evidence.

New behavior begins in the simplest working application location. `sdk` is not
a staging area for speculative abstractions: move code there only after a
working implementation demonstrates reusable Raylib-specific policy. Extract
`app` modules when cohesive data and lifecycle ownership have become visible in
real code. Refactoring is therefore a later checkpoint outcome, not a condition
for beginning a feature.

Checkpoint card labels follow this order:

- **Challenge** and **Visible finish** define what to build.
- **Hints** and **Read** are just-in-time support, not required pre-reading.
- **Reflect after** is discussed only after the attempt runs.
- **Self-check** is part of review, not homework before coding.

When you ask for help, I will use this hint ladder:

1. **Question:** help you identify the missing concept.
2. **Directional hint:** point toward the relevant data or stage.
3. **API hint:** name useful Raylib/C++ functions or types.
4. **Structural hint:** suggest an interface shape or invariant.
5. **Worked fragment:** only after you have attempted it or explicitly request
   more direct help.

The goal is productive experimentation, not ceremonial suffering. If a
checkpoint has stopped teaching and has started consuming your afternoon, say
so and we will increase the specificity of the next hint.

## Definition of a complete checkpoint

A checkpoint is complete when all of these are true:

- `./build` succeeds with no new warnings.
- The executable launches and displays the stated visible result.
- Closing the window exits normally.
- Previously completed controls still work.
- Review finds no correctness or ownership issue that blocks later work.
- After implementation, you can explain the main data entering and leaving the
  new behavior.
- You have made a small checkpoint commit.

At the final integration checkpoint, relevant automated tests must also pass.

## Rules for using the baseline

The `baseline` tag is a reference implementation, not starter code.

Before completing a checkpoint, you may freely inspect:

- `REPORT.md` for concepts;
- Raylib headers, examples, and vendored implementation notes;
- official documentation linked below;
- your own earlier commits.

After your checkpoint works, you may compare structure with `baseline`. Prefer a
summary first:

```powershell
git diff --stat baseline -- src
```

Then narrow comparison to the module you just completed. Avoid opening the
reference implementation of a function before attempting your own unless you
are genuinely blocked or intentionally studying alternatives.

Never switch the working tree to `baseline` over uncommitted work. The tag is
read-only and can be inspected with Git commands.

## Final constraints to practice from the beginning

- C++11 with plain structs and free functions.
- Explicit for-loops; no iterator-style loops.
- No application classes, inheritance, exceptions, STL containers, smart
  pointers, templates, or lambdas.
- References for required borrowed inputs; pointers for real memory ranges,
  optional memory, callbacks, C strings, and pointer-plus-count streams.
- Safe all-zero public state and explicit initialization/shutdown.
- Integer result codes for expected failures.
- One-based handles for stable array identities; zero is a harmless stub.
- Caller-selected allocation at ownership boundaries.
- Immediate-mode rendering; no retained application scene in the SDK.
- No changes anywhere under `vendor/raylib`.

Early checkpoints will not use every final rule immediately. When a later step
introduces a rule, refactor the smallest affected surface and keep the visual
result alive.

---

# Phase 1 — First pixels and a trustworthy loop

The first phase proves the toolchain and establishes the frame lifecycle before
introducing 3D.

## Checkpoint 1 — A software-rendered window

**Challenge:** Make the reduced project compile and open one Raylib window using
the existing software-renderer build configuration.

**Visible finish:** A solid-color 1920×1080 window opens, remains responsive,
and closes cleanly.

**Reflect after:** Which build declarations describe a target, its current source
files, and its Raylib dependency? What has become stale since `src/` was
deleted?

**Hints:** Start with one source file. Keep `PLATFORM=RGFW` and
`OPENGL_VERSION=Software`. Raylib's basic-window example is a map of the runtime
lifecycle, not code you need to copy wholesale.

**Read:** [R1], [R2], [R4], and the local
`vendor/raylib/examples/core/core_basic_window.c`.

**Self-check:** Confirm the startup log names `RLSW OpenGL Software Renderer`.

## Checkpoint 2 — A frame you can see changing

**Challenge:** Introduce a small update value that changes over time and affects
the background or a simple 2D shape.

**Visible finish:** The window visibly pulses, moves, or changes color while
remaining responsive.

**Reflect after:** What belongs before drawing, between frame begin/end, and after
the loop? Why must every frame clear the previous image?

**Hints:** Look for Raylib's frame-time or elapsed-time query. Clamp or wrap the
value so the display remains intentional after running for several minutes.

**Read:** [R1] and Raylib core examples involving frame timing.

**Self-check:** Drag and resize the window; animation should resume normally.

## Checkpoint 3 — A diagnostic overlay

**Challenge:** Display enough 2D text to prove frame rate and basic controls
without using custom fonts yet.

**Visible finish:** FPS, a short title, and an exit hint appear over the animated
background.

**Reflect after:** Why is an on-screen diagnostic more useful than printing every
frame to the terminal?

**Hints:** Raylib has a direct FPS helper and a default font. Keep diagnostics
outside any future 3D camera block.

**Read:** [R1] and the `text` section of [R2].

**Self-check:** Text remains screen-aligned as the background changes.

## Checkpoint 4 — Extract the runtime boundary

**Challenge:** Move window/frame lifecycle behavior into the first `sdk` module
without changing what appears on screen.

**Visible finish:** The same animation and overlay run; the window title should
identify this as the SDK-runtime checkpoint.

**Reflect after:** What Raylib details are generic to this project, and what state
still belongs to the application? What should an all-zero runtime configuration
mean?

**Hints:** Prefer a small configuration struct and free functions. Keep
`main.cpp` responsible for sequencing and ownership decisions.

**Read:** `REPORT.md`, sections 6 and 7.

**Self-check:** Initialization failure must prevent entry into the frame loop.

## Checkpoint 5 — Enter a 3D world

**Challenge:** Add a fixed perspective camera and simple world landmarks.

**Visible finish:** A stationary 3D grid and three colored axis lines are
visible beneath the 2D overlay.

**Reflect after:** What do camera position, target, up, field of view, and
projection each control? Which drawing calls belong inside 3D mode?

**Hints:** Begin with an asymmetric camera position so X, Y, and Z are visually
distinguishable. Use the conventional project colors: X blue, Y red, Z green.

**Read:** [R1], [R2], and
`vendor/raylib/examples/core/core_3d_camera_mode.c`.

**Self-check:** Change one camera component at a time and predict the result
before running.

### Phase 1 gate

You should now be able to explain the outer frame loop and the role of
`BeginMode3D` without mentioning cubes yet.

---

# Phase 2 — One cube and an orbit camera

This phase builds the smallest useful 3D inspection tool.

## Checkpoint 6 — One visible cube

**Challenge:** Place one filled cube with a contrasting wireframe at the world
origin.

**Visible finish:** The camera clearly shows a solid cube, its twelve edges, the
grid, and axis landmarks.

**Reflect after:** Is the position the cube's center or a corner? How does edge
length relate to its bounding box?

**Hints:** Compare a complete cube draw with its wireframe companion. Choose a
size that leaves visible space above the grid.

**Read:** The model-shapes portion of [R1] and [R2].

**Self-check:** Temporarily move the cube along each axis and identify the screen
direction that results.

## Checkpoint 7 — Derive an orbit camera

**Challenge:** Represent the camera as target, yaw, pitch, and distance, then
derive the Raylib camera from that state every frame.

**Visible finish:** A time-driven or keyboard-driven yaw circles the stationary
cube while always looking at its center.

**Reflect after:** Which trigonometric components produce horizontal radius and
vertical height? Why is derived `Camera3D` state preferable to maintaining two
independent camera representations?

**Hints:** Draw a right triangle for pitch before writing code. Keep yaw and
pitch in radians.

**Read:** [R5] and `REPORT.md`, section 3.8.

**Self-check:** At zero pitch, camera height should match target height. At
positive pitch it should move above the target.

## Checkpoint 8 — Mouse orbit and zoom

**Challenge:** Drive yaw/pitch from mouse delta and orbit distance from the
mouse wheel.

**Visible finish:** Mouse motion rotates smoothly around the cube; wheel input
zooms without passing through the target or escaping uselessly far away.

**Reflect after:** Why must pitch and distance be clamped? Why should sensitivity
not depend on absolute cursor position?

**Hints:** Search Raylib for mouse delta rather than deriving it from two cursor
positions. Test inverted signs experimentally and document your convention.

**Read:** [R1] and local camera examples under `vendor/raylib/examples/core`.

**Self-check:** Rapid input at both zoom limits must keep the camera valid.

## Checkpoint 9 — Cursor capture without jumps

**Challenge:** Capture the cursor for orbiting, toggle capture with right-click,
and remain stable across focus changes.

**Visible finish:** Captured mode rotates freely; uncaptured mode exposes a
normal pointer; toggling does not fling the camera.

**Reflect after:** What mouse delta appears on the first frame after recapture?
What happens if the window loses focus while logically captured?

**Hints:** Use Raylib's cursor functions rather than implementing recentering.
Treat “suppress one delta” as explicit camera state.

**Read:** [R3] and the cursor portion of [R1].

**Self-check:** Toggle while moving the mouse and alt-tab in both states.

## Checkpoint 10 — A reusable orbit-camera SDK module

**Challenge:** Extract the orbit behavior into `sdk/orbit_camera` while the
single-cube app remains unchanged.

**Visible finish:** The cube interaction is identical; the overlay reports
target, yaw, pitch, distance, and capture state from the new module.

**Reflect after:** Which inputs should be sampled by the app versus interpreted by
the camera? Which state is persistent and which `Camera3D` value is derived?

**Hints:** Keep cube knowledge out of the SDK. Required configuration/state
inputs are good candidates for references.

**Read:** `REPORT.md`, sections 6.2, 6.3, and 10.

**Self-check:** The all-zero camera must safely produce an inert result.

### Phase 2 gate

Sketch the data flow from mouse delta to final camera position. If you cannot
explain each value, pause here and ask for a conceptual review.

---

# Phase 3 — From one cube to data-driven fields

This phase replaces hard-coded geometry with compact field data. Features are
built directly in the working application first; module extraction happens at
the end only after the code exposes real boundaries.

## Checkpoint 11 — A tiny cube field

**Challenge:** Describe a small field with dimensions, cube size, spacing,
and origin; render it directly in the working application using explicit nested
loops. Do not extract a field or application module yet.

**Visible finish:** A centered `3 × 3 × 3` cube field appears with visible gaps.

**Reflect after:** Which coordinate ended up as the innermost loop? Which values
were identical for every cube?

**Hints:** Begin with integer grid coordinates. Derive world centers during the
loop instead of storing them. It is acceptable for the first working version to
live in `main.cpp`.

**Read:** `REPORT.md`, sections 8.1 and 8.5.

**Self-check:** Increase one dimension only and identify its world axis.

## Checkpoint 12 — Centering and implicit positions

**Challenge:** Make arbitrary field dimensions center correctly around the
world origin without a per-cube position array.

**Visible finish:** Switch among odd and even dimensions; the field remains
geometrically centered on the axis landmarks.

**Reflect after:** A row of N centers contains how many intervals? Where did the
first center land relative to the origin?

**Hints:** Get one axis working in code, draw it, then reuse the same relationship
for the other two axes.

**Read:** `REPORT.md`, section 8.1.

**Self-check:** Test dimensions 1, 2, 3, and 4 independently.

## Checkpoint 13 — One-based cube handles

**Challenge:** Convert between `(x,y,z)` and a one-based 32-bit handle while
reserving zero as “no cube.”

**Visible finish:** The overlay shows the handle and recovered coordinate for a
visually highlighted cube; changing the highlighted coordinate preserves the
round trip.

**Reflect after:** How did you flatten the coordinates? Where did you add or
remove the reserved stub offset?

**Hints:** First derive the zero-based flattening formula. Add the handle rule
only at the identity boundary.

**Read:** `REPORT.md`, section 8.2, and [R6].

**Self-check:** Test first, last, and out-of-bounds coordinates plus handle zero.

## Checkpoint 14 — Cube values A through D

**Challenge:** Define a zero value and A/B/C/D categories, then associate one
category with each real cube.

**Visible finish:** The tiny field displays four clearly different fill colors,
and the overlay identifies the highlighted cube's value.

**Reflect after:** What changed when cubes began storing category bytes instead
of colors? What did the zero slot simplify?

**Hints:** Use a simple provisional pattern first. Keep position and identity
implicit; only category needs per-cube storage.

**Read:** `REPORT.md`, sections 8.2 and 8.3.

**Self-check:** Invalid handles resolve to the zero value without exposing the
backing pointer as identity.

## Checkpoint 15 — Deterministic value generation

**Challenge:** Replace the provisional pattern with a coordinate-derived hash
that produces varied A/B/C/D data and repeats across runs.

**Visible finish:** A larger field appears visually mixed; restarting reproduces
the exact same pattern.

**Reflect after:** Where did visible stripes appear in weaker attempts? What did
the avalanche step change?

**Hints:** Treat generation as startup work. Hash coordinates, then map a small
unbiased result range onto the enum.

**Read:** `REPORT.md`, section 8.3.

**Self-check:** Sample coordinates separated by powers of two and inspect their
distribution.

## Checkpoint 16 — Palette-defined fill and edges

**Challenge:** Separate semantic value from visual style and introduce three
palettes containing adjacent fill and edge colors.

**Visible finish:** Keys 1/2/3 recolor every visible cube and its wireframe
without changing cube values. D is white at roughly 60% opacity in each palette.

**Reflect after:** Why did keeping fill and edge colors adjacent help? What
remained unchanged when the active palette changed?

**Hints:** Give palette zero an all-zero style table. Resolve invalid palette or
value handles to their stubs.

**Read:** `REPORT.md`, section 8.4.

**Self-check:** Log or overlay one cube's value before and after palette changes;
the value must not change.

## Checkpoint 17 — Refactor proven application boundaries

**Challenge:** Now that field configuration, cube identity, values, generation,
palettes, camera control, update behavior, and rendering all exist, identify the
boundaries that the working code is asking for. Extract only those proven
responsibilities into `app` modules and keep reusable Raylib policy in `sdk`.

**Visible finish:** The same colored field and controls run, while `main.cpp`
becomes a small composition root with explicit initialization, loop, and
shutdown.

**Reflect after:** Which extraction removed real duplication or ownership
confusion? Which possible abstraction still lacked enough evidence to create?

**Hints:** Move cohesive working code rather than redesigning it. Preserve data
layout and call order first; improve names and interfaces only after behavior is
unchanged.

**Read:** `REPORT.md`, sections 6 and 16, after the first extraction attempt.

**Self-check:** Compare the pre-refactor and post-refactor executable behavior,
then force one startup failure and confirm shutdown remains safe.

### Phase 3 gate

You should now be able to estimate the bytes required for 50 million categories
and explain why positions are not part of that estimate.

---

# Phase 4 — Focused view and navigation

This phase turns the field into an inspection application.

## Checkpoint 18 — Selected-cube state

**Challenge:** Add an application view controller containing a selected handle
and focused mode. Target the orbit camera at the selected cube.

**Visible finish:** One cube is visibly emphasized, the camera orbits its exact
center, and the overlay shows its handle/coordinate/value.

**Reflect after:** Why should the controller store a handle rather than a cube
pointer or duplicate position?

**Hints:** Start by selecting the field's center coordinate during controller
initialization.

**Read:** `REPORT.md`, sections 8.2 and 10.

**Self-check:** Snapping between two known handles changes target but preserves
camera yaw and pitch.

## Checkpoint 19 — A bounded focused region

**Challenge:** Render only coordinates inside a radius-sized region around the
selected cube, clamped to field bounds.

**Visible finish:** A local cube neighborhood follows the selection; selecting
near an edge produces a clipped neighborhood without invalid memory access.

**Reflect after:** Why compute minimum/maximum once before the hot loops? Where can
unsigned subtraction underflow?

**Hints:** Use wider arithmetic when adding a radius near the maximum coordinate.

**Read:** `REPORT.md`, section 12.2.

**Self-check:** Exercise all eight field corners.

## Checkpoint 20 — Euclidean radius culling

**Challenge:** Turn the local box into a radius-five sphere in grid space.

**Visible finish:** Box corners disappear, leaving a rounded lattice volume of
roughly 515 cubes away from boundaries.

**Reflect after:** Why compare squared distance instead of taking a square root?
Which differences need signed or wider types?

**Hints:** Keep the clamped box as the enumeration bound; add a cheap acceptance
test inside it.

**Read:** `REPORT.md`, sections 12.2 and 13.

**Self-check:** Overlay the submitted cube count and predict how boundaries
change it.

## Checkpoint 21 — One semantic input snapshot

**Challenge:** Poll Raylib once per frame into semantic actions and analog mouse
values consumed by the app/controller.

**Visible finish:** Existing camera and palette controls behave identically; an
overlay can briefly list action names as they fire.

**Reflect after:** What goes wrong when two systems independently consume mouse
delta? Which controls are edge-triggered versus continuous?

**Hints:** A bit mask suits simultaneous button actions; mouse delta and wheel
remain ordinary values.

**Read:** `REPORT.md`, section 10.

**Self-check:** Press multiple compatible controls in one frame and observe all
relevant action bits.

## Checkpoint 22 — Face-adjacent world navigation

**Challenge:** Move the selection by one valid neighboring cube using
WASD/arrows, initially in fixed world directions.

**Visible finish:** Each key snaps the target exactly one cube, never leaving the
field.

**Reflect after:** Is navigation changing a world position, a coordinate, or a
handle? Which representation makes bounds checking simplest?

**Hints:** Resolve handle to coordinate, alter one axis, validate, then resolve
back to a handle.

**Read:** `REPORT.md`, section 10.

**Self-check:** Hold each direction at every field boundary.

## Checkpoint 23 — Camera-relative horizontal navigation

**Challenge:** Make left/right/forward/back follow camera orientation instead of
fixed world axes.

**Visible finish:** After rotating around the target, navigation still moves in
the direction that looks left/right/forward/back on screen.

**Reflect after:** What vectors describe camera forward and right? How do you map
a continuous direction onto one discrete grid axis without diagonal movement?

**Hints:** Ignore vertical contribution for the first version. Compare dominant
absolute components and preserve their signs.

**Read:** [R5] and `REPORT.md`, section 10.

**Self-check:** Test near 0°, 45°, 90°, and 180° yaw.

## Checkpoint 24 — Vertical navigation at steep pitch

**Challenge:** Allow forward/back to traverse field Y when the camera looks
steeply downward or upward.

**Visible finish:** Near-horizontal views navigate the XZ plane; steep views
move between cube layers in the direction that feels screen-relative.

**Reflect after:** What threshold makes vertical intent dominant? How should
looking up versus down affect the sign?

**Hints:** Base the decision on the camera forward vector, not raw mouse input.

**Read:** `REPORT.md`, sections 5 and 10.

**Self-check:** Test both positive and negative steep pitch near top/bottom field
boundaries.

## Checkpoint 25 — Ray picking in the focused neighborhood

**Challenge:** While the cursor is free, convert a click into a world ray and
select the nearest intersected local cube.

**Visible finish:** Right-click releases the cursor; left-clicking a visible cube
snaps the target to exactly that cube and recaptures the cursor.

**Reflect after:** Why must all candidates be tested before selecting? What does
collision distance solve when boxes overlap on screen?

**Hints:** Test only the same local candidates used by drawing. Derive each
bounding box from center and half-size.

**Read:** [R3], [R7], and local
`vendor/raylib/examples/core/core_3d_picking.c`.

**Self-check:** Click a foreground cube that visually overlaps a background
cube; the nearer one must win.

### Phase 4 gate

The focused view should now be usable even before labels and compasses exist.
Commit a short screen recording or screenshot alongside your notes if useful.

---

# Phase 5 — Faces, text, compasses, and boundaries

This phase teaches face-local coordinate systems and camera-facing annotation.

## Checkpoint 26 — Canonical axis-direction metadata

**Challenge:** Define one table for ±X, ±Y, and ±Z containing normal, face-local
right/down vectors, axis identity, and short label.

**Visible finish:** The selected cube draws six small colored normal lines or
face markers, each labeled in the 2D overlay.

**Reflect after:** Why should rendering, text, compass, and edge orientation share
one direction definition? What must direction zero contain?

**Hints:** Verify one face at a time. Use the right-hand rule consistently.

**Read:** `REPORT.md`, sections 12.5–12.7.

**Self-check:** From outside each face, local right/down should orient text
consistently rather than mirror it.

## Checkpoint 27 — Persistent font resources

**Challenge:** Load the supplied Fira Code font during initialization and unload
it during shutdown.

**Visible finish:** Replace the default overlay font with the loaded font and
show an ownership/resource-ready diagnostic.

**Reflect after:** Why is font loading startup work? How does partial
initialization affect shutdown?

**Hints:** Keep Raylib-owned resources in one zero-initializable SDK bundle with
explicit ownership flags.

**Read:** [R1], [R2], and `REPORT.md`, sections 9.1 and 12.6.

**Self-check:** Temporarily use a bad font path and verify failure cleanup.

## Checkpoint 28 — One line of text on one cube face

**Challenge:** Construct glyph quads on one selected cube face using that face's
local right/down basis.

**Visible finish:** One readable label lies just above a cube surface rather than
floating as screen text.

**Reflect after:** How do font pixels become world units? Why does the text need a
small surface offset? What creates mirrored text?

**Hints:** Solve placement with one short fixed string before dynamic handles.
Study Raylib's font rectangles and glyph advances.

**Read:** [R1], local text examples, and `REPORT.md`, section 12.6.

**Self-check:** Orbit around the cube and confirm the label remains attached to
the intended face.

## Checkpoint 29 — Complete local face text

**Challenge:** Draw handle, value label, and direction on appropriate faces for
cubes within radius three of the selection.

**Visible finish:** Nearby front-facing cube faces show three compact text lines;
distant local cubes and back-facing faces do not.

**Reflect after:** Which text widths repeat and can be measured once? How does a
face normal dot camera direction identify a back face?

**Hints:** Submit fills before text. Keep the dynamic handle formatting local to
the draw loop without allocating strings on the heap.

**Read:** `REPORT.md`, sections 12.2 and 12.6.

**Self-check:** Overlay the number of labeled faces and verify it drops when
looking from another side.

## Checkpoint 30 — One compass arrow primitive

**Challenge:** Create an SDK arrow from a shaft and arrowhead, independent of
cube-field knowledge.

**Visible finish:** A clearly proportioned +X arrow extends from the selected
cube.

**Reflect after:** What inputs make the primitive reusable at both local and
field scale? Which vector should be normalized?

**Hints:** Raylib's cylinder-between-points operation can represent both a
cylinder and a cone.

**Read:** The 3D shapes section of [R1].

**Self-check:** Point the same primitive in non-axis-aligned directions during a
temporary experiment.

## Checkpoint 31 — Six colored compass directions

**Challenge:** Use canonical direction metadata to draw arrows from all six
selected-cube faces.

**Visible finish:** ±X are blue, ±Y red, and ±Z green, each beginning just beyond
the selected cube face.

**Reflect after:** Which dimensions are view configuration versus direction data?
Why should positive and negative directions share an axis color?

**Hints:** Let the app choose placement/scale and the SDK choose primitive
emission.

**Read:** `REPORT.md`, section 12.7.

**Self-check:** No arrow should begin inside the cube or point toward its center.

## Checkpoint 32 — Billboard direction labels

**Challenge:** Rasterize six fixed labels once into an atlas, then draw the
correct label facing the camera beside each arrow.

**Visible finish:** White `+x/-x/+y/-y/+z/-z` labels remain readable through yaw
and pitch.

**Reflect after:** Why use one atlas instead of six frame-generated textures? Why
is world Y insufficient as billboard up when the camera pitches?

**Hints:** Derive view-up from the camera view basis. Direction zero should map
to an empty atlas rectangle.

**Read:** [R1] and `REPORT.md`, sections 12.6 and 12.7.

**Self-check:** Orbit near the pitch limit and watch for label roll or flattening.

## Checkpoint 33 — Shaft gaps around labels

**Challenge:** Split each arrow shaft around a centered label gap while keeping
the arrowhead intact.

**Visible finish:** No shaft passes through label text; both local-scale segments
remain visually continuous around the gap.

**Reflect after:** Which distances must be clamped when the requested gap is
larger than the available shaft?

**Hints:** Think in scalar distances along a normalized direction before
constructing segment endpoints.

**Read:** `REPORT.md`, section 12.7.

**Self-check:** Try zero, oversized, and off-center gaps without invalid
geometry.

## Checkpoint 34 — Local boundary grid patches

**Challenge:** When the selection approaches a field edge, draw a bounded grid
patch on that outer plane.

**Visible finish:** Moving near ±X/±Y/±Z boundaries reveals a local grid patch;
interior selections show none.

**Reflect after:** How do two step vectors define a rectangular plane? Why are
bounded patches preferable to entire 500×1000 surfaces?

**Hints:** Treat cell counts and line counts carefully: N cells require N+1
lines.

**Read:** `REPORT.md`, sections 12.7 and 13.

**Self-check:** Corners may show three patches, with aligned borders and no
out-of-field indexing.

### Phase 5 gate

At this point the local view should resemble the final experience. Take time to
clean naming and explain the boundary between app decisions and SDK primitives.

---

# Phase 6 — Explicit ownership and immediate rendering

This phase revises memory and interfaces while preserving the local scene.

## Checkpoint 35 — Immediate draw packets

**Challenge:** Express reusable SDK operations as compact value draw structs
consumed synchronously by free functions.

**Visible finish:** The local scene remains unchanged; the overlay reports the
number of immediate cube/face/text/arrow/grid submissions.

**Reflect after:** Which values completely describe one draw? What would make the
SDK accidentally retained-mode? Which data belongs to the app instead?

**Hints:** A draw function must not store packet pointers after return. Dense
arrays may still be passed as pointer plus count.

**Read:** `REPORT.md`, section 12.1.

**Self-check:** Every draw packet can be stack-allocated and discarded
immediately after the call.

## Checkpoint 36 — Render data versus frame data

**Challenge:** Separate persistent render resources/data from a value-only
snapshot of camera, selected handle, and view mode.

**Visible finish:** Rendering and picking agree on the same camera; an overlay
shows a monotonically increasing frame number from orchestration, not retained
renderer state.

**Reflect after:** Which data changes every frame? Which data is immutable? Why
should the renderer not own the view controller?

**Hints:** Have the app compose a frame packet after update and before drawing.

**Read:** `REPORT.md`, sections 6.3 and 7.

**Self-check:** Renderer code should not poll input or mutate selection.

## Checkpoint 37 — A caller-supplied allocator interface

**Challenge:** Define a small allocation interface carrying context and aligned
allocate/release callbacks; make zero state inert.

**Visible finish:** The app still runs, and a startup/overlay diagnostic confirms
the allocator is valid and reports persistent bytes acquired.

**Reflect after:** Why pass size and alignment to release? Why is callback
indirection acceptable at ownership boundaries but not per cube?

**Hints:** Build a default aligned-heap adapter first. Keep allocation policy out
of cube-field functions.

**Read:** `REPORT.md`, sections 9.1 and 9.2, plus [R8].

**Self-check:** A zero allocator returns no memory and release does nothing.

## Checkpoint 38 — Persistent immutable cube data

**Challenge:** Allocate the value array through the supplied persistent
allocator, align it to a cache-line boundary, generate once, and release through
the same policy.

**Visible finish:** The local field looks identical; the overlay reports value
capacity, approximate MiB, and alignment.

**Reflect after:** Why does capacity include the stub? Why does aligned storage not
justify scanning the entire dataset each frame?

**Hints:** Allocation and generation belong at initialization, not rendering.
Use assertions or diagnostics to inspect address alignment.

**Read:** [R8], [R9], and `REPORT.md`, sections 8.5 and 9.

**Self-check:** Invalid handles still resolve safely after the ownership change.

## Checkpoint 39 — A non-owning frame arena

**Challenge:** Bind a linear arena to caller-owned memory, support aligned
allocation, expose it through `Allocator`, and reset it after each frame.

**Visible finish:** Allocate a small per-frame diagnostic buffer from the arena;
display used bytes and preserved high-water mark while the scene runs.

**Reflect after:** What pointer lifetimes end at reset? Why should individual arena
release be a no-op? What overflow checks does alignment rounding require?

**Hints:** The arena should never acquire its own backing memory and should never
fall back to the heap.

**Read:** `REPORT.md`, section 9.3.

**Self-check:** Deliberately request too much and observe a controlled failure,
then restore capacity.

## Checkpoint 40 — Query frame memory before startup

**Challenge:** Let the renderer calculate its worst-case temporary command
capacity from public configuration before the application allocates its arena.

**Visible finish:** Startup displays required scratch bytes; per-frame high-water
use stays within that bound.

**Reflect after:** What focused and birds-eye workloads determine the maximum?
Which arithmetic products can overflow before conversion to `size_t`?

**Hints:** Requirement functions describe memory but do not allocate it. The
composition root chooses backing storage.

**Read:** `REPORT.md`, sections 7, 9, and 13.

**Self-check:** Increasing focused radius increases the query predictably or is
rejected safely.

### Phase 6 gate

Draw a lifetime diagram showing persistent allocator, arena backing, frame
allocations, Raylib-owned resources, and stack packets. Review it with me before
starting transparency.

---

# Phase 7 — Transparency and correct draw ordering

This phase deliberately exposes a rendering problem before solving it.

## Checkpoint 41 — Observe naive transparency

**Challenge:** Render D using its 60%-opaque white fill and study the result from
several angles.

**Visible finish:** Transparent D cubes are visible, including any incorrect
faces or ordering artifacts you can reproduce and describe.

**Reflect after:** What does alpha blending combine? Why can opaque depth logic
produce surprising transparent results? Does a whole-cube helper issue color in
the way `rlsw` expects?

**Hints:** This checkpoint is successful when you can name the failure, not when
you have already hidden it.

**Read:** [R10], [R11], [R12], and `REPORT.md`, sections 3.6–3.7.

**Self-check:** Capture screenshots with both stopped and moving camera to
distinguish persistent geometry artifacts from temporal tearing.

## Checkpoint 42 — Explicit six-face transparent cubes

**Challenge:** Make all six D faces receive consistent alpha under `rlsw`
without changing vendor code.

**Visible finish:** No cube has only one transparent triangle/face while the
rest remains opaque.

**Reflect after:** At what primitive boundary does color/alpha state need to be
reissued? Why is a six-quad application workaround acceptable here?

**Hints:** Inspect the local `rlsw` behavior only to understand the contract;
keep the fix in project-owned SDK rendering code.

**Read:** `REPORT.md`, sections 12.4 and 14, and local
`vendor/raylib/src/external/rlsw.h` as a diagnostic reference.

**Self-check:** Verify all face directions and multiple D cubes.

## Checkpoint 43 — Back-to-front transparent commands

**Challenge:** Collect transparent face commands in frame memory, assign camera
depth, sort far-to-near, and submit after opaque fills with depth writes off.

**Visible finish:** Overlapping transparent cubes blend consistently while
opaque cubes still occlude them correctly.

**Reflect after:** Why is the command array still immediate-mode? Why sort faces
rather than cube centers? Why retain depth testing but disable depth writes?

**Hints:** Use an in-place algorithm over the arena array; no STL container or
second command buffer is needed.

**Read:** [R10], [R11], [R12], and `REPORT.md`, section 12.4.

**Self-check:** Render a controlled near/far pair whose expected color layering
you can explain.

### Phase 7 gate

You should now be able to explain color-buffer blending separately from
depth-buffer testing and writing. This distinction is foundational.

---

# Phase 8 — Birds-eye mode

This phase introduces a second representation of the same source data.

## Checkpoint 44 — A second camera mode

**Challenge:** Add birds-eye mode toggled by `G`, with its own orbit/zoom preset
and a target at field center.

**Visible finish:** `G` switches between focused cubes and a distant view of a
wireframe field-bounds box; camera rotation and wheel zoom work in both.

**Reflect after:** Which controller state is shared between modes? Which camera
settings differ? Should switching reconstruct the controller or apply a preset?

**Hints:** Keep selected cube identity while changing target/preset. Use the
field's half extents to reason about distance and far clipping.

**Read:** `REPORT.md`, sections 5, 10, and 12.3.

**Self-check:** Repeated toggles never lose capture state or accumulate camera
errors.

## Checkpoint 45 — Exterior shell on a small field

**Challenge:** On a deliberately small field, enumerate only cubes belonging to
the six exterior surfaces.

**Visible finish:** Birds-eye mode shows a hollow colored shell; focused mode is
unchanged.

**Reflect after:** How can face loops avoid submitting corner/edge cells multiple
times? Why is rendering the whole small volume useful only as a temporary
comparison?

**Hints:** Count expected shell cells on paper. Let one pair of faces own its
full rectangle and later pairs exclude previously owned boundaries.

**Read:** `REPORT.md`, section 12.3.

**Self-check:** Overlay submitted shell count and compare it with your manual
formula.

## Checkpoint 46 — Endpoint-preserving coarse sampling

**Challenge:** Build a virtual lattice that samples source axes approximately
every 32 cells while always representing both endpoints.

**Visible finish:** Birds-eye mode shows a dramatically coarser shell whose
opposite outer faces still align with the intended field bounds.

**Reflect after:** Why does ordinary integer stepping often miss the last source
coordinate? How do sample count and interval count differ?

**Hints:** Separate virtual sample coordinates from original source coordinates.
Map endpoints exactly and distribute interior samples evenly using wide
intermediate arithmetic.

**Read:** `REPORT.md`, sections 8.3 and 12.3.

**Self-check:** Test source dimensions smaller than, equal to, and larger than
the stride.

## Checkpoint 47 — Non-overlapping representative geometry

**Challenge:** Size and place virtual representatives so neighbors touch exactly
without gaps or overlap, independent of source aspect ratio.

**Visible finish:** The coarse shell looks like one continuous box with no
interpenetrating cubes or inset outer edge.

**Reflect after:** Which distance should equal representative edge length? Why
should visual lattice placement be separate from sampled source placement?

**Hints:** Derive representative spacing from source stride and field spacing,
then center the virtual lattice from its sample counts.

**Read:** `REPORT.md`, section 12.3.

**Self-check:** Inspect corners while rotating and while stationary.

## Checkpoint 48 — Source values and birds-eye picking

**Challenge:** Give every representative the value of its mapped source cube and
allow uncaptured clicks to select that source identity.

**Visible finish:** The shell shows a mixed deterministic palette; clicking a
representative enters focused mode on the corresponding real cube.

**Reflect after:** Why must drawing and picking share the exact same lattice math?
What identity should a virtual representative return?

**Hints:** Do not create a retained representative array. Re-enumerate the same
bounded shell for draw and pick using common mapping functions.

**Read:** [R3], [R7], and `REPORT.md`, sections 11 and 12.3.

**Self-check:** Pick all six faces, including endpoint representatives.

## Checkpoint 49 — Outward faces and unique edges

**Challenge:** Replace birds-eye complete cubes with outward face commands and
assign every visible shell edge to exactly one face.

**Visible finish:** No interior/coincident faces are drawn, shared lines do not
fight, and the shell remains stable when rotation stops.

**Reflect after:** Which four local edge bits describe a face? How do coplanar
neighbors and perpendicular shell faces decide ownership deterministically?

**Hints:** First remove internal faces. Then solve coplanar shared lines. Finally
solve the twelve boundaries where shell planes meet. Inspect one corner at a
time.

**Read:** [R13] and `REPORT.md`, sections 12.5 and 14.

**Self-check:** Use contrasting palette edge colors and examine every corner
while stationary.

## Checkpoint 50 — Field-scale compass and final integration

**Challenge:** Add the large birds-eye compass, switch to the final
`500 × 1000 × 100` field, validate performance, and add nonvisual regression
tests for the core invariants accumulated throughout the curriculum.

**Visible finish:** The complete application matches the reference behavior:
focused and birds-eye views, navigation, picking, palettes, transparency, text,
compasses, boundary grids, and 30+ FPS software rendering.

**Reflect after:** Which visual features scale with selected cube, virtual shell,
camera distance, or screen resolution? Which mathematical rules deserve tests?
Where is the actual frame time spent?

**Hints:** Add tests for zero state, arena alignment/exhaustion/reset, handle
round trips, stub resolution, sampling endpoints, region clipping, scratch
requirements, and steep-pitch navigation. Profile before changing algorithms.

**Read:** `REPORT.md`, sections 13–20, [R9], and [R14].

**Self-check:** Run all controls, CTest, a clean build, `git diff --check`, and a
vendor diff. Compare your module interfaces and behavior with `baseline`, then
write down where your design intentionally differs.

### Final gate

You are finished when you can rebuild the architecture from your own reasoning,
not when your source is textually identical to `baseline`.

Prepare a short retrospective answering:

1. Which performance wins came from drawing less rather than drawing faster?
2. Which bugs were really coordinate-space misunderstandings?
3. Which interfaces made ownership easier to reason about?
4. What did zero stubs and handles simplify?
5. Why is the renderer immediate-mode even though it temporarily sorts commands?
6. Where would a GPU version differ, and where would application logic remain
   the same?

---

# Resource index

## Project-local resources

- `REPORT.md` — finished architecture and Graphics Rendering 101.
- `vendor/raylib/src/raylib.h` — exact vendored public API declarations.
- `vendor/raylib/src/raymath.h` — exact vector/matrix helpers.
- `vendor/raylib/examples/` — examples matching the installed vendor version.
- `vendor/raylib/src/external/rlsw.h` — software backend, for diagnosis only.
- Git tag `baseline` — completed reference implementation.

## Official external resources

**[R1] Raylib cheatsheet**  
<https://www.raylib.com/cheatsheet/cheatsheet.html>

Use this to discover function names and data structures. Confirm signatures
against the vendored `raylib.h`, because this project uses a development build.

**[R2] Raylib examples collection**  
<https://www.raylib.com/examples>

Raylib is intentionally taught through examples. Search by function name, then
reduce the example to the concept you are currently studying.

**[R3] Raylib 3D picking example**  
<https://www.raylib.com/examples/core/loader.html?name=core_3d_picking>

Focus on the relationship among mouse position, world ray, bounding box,
collision distance, and cursor mode.

**[R4] CMake tutorial**  
<https://cmake.org/cmake/help/latest/guide/tutorial/index.html>

The first step is enough for the early curriculum. Use the command references
when working with vendored Raylib targets.

**[R5] Raymath source/reference**  
<https://github.com/raysan5/raylib/blob/master/src/raymath.h>

Study vector normalization, cross/dot products, matrices, and camera-related
helpers. Prefer the local vendored copy for exact behavior.

**[R6] Microsoft C++ language reference**  
<https://learn.microsoft.com/en-us/cpp/cpp/cpp-language-reference?view=msvc-170>

Use selectively for fixed-width arithmetic, references, enums, structs, and
conversion rules encountered in the project.

**[R7] Raylib mesh-picking example**  
<https://www.raylib.com/examples/models/loader.html?name=models_mesh_picking>

This demonstrates selecting the nearest collision among different candidates.

**[R8] Microsoft `alignas` reference**  
<https://learn.microsoft.com/en-us/cpp/cpp/alignas-specifier?view=msvc-170>

Read this before choosing explicit type/object alignment. Alignment is a
contract, not a universal performance button.

**[R9] Intel optimization reference manual**  
<https://www.intel.com/content/www/us/en/content-details/671488/intel-64-and-ia-32-architectures-optimization-reference-manual-volume-1.html>

Use as a deeper reference for cache locality and memory performance. Do not try
to read it cover to cover during an early checkpoint.

**[R10] Khronos depth-test guide**  
<https://wikis.khronos.org/opengl/Depth_Test>

Although `rlsw` is software, its `rlgl` state model follows these familiar depth
concepts.

**[R11] Khronos blending guide**  
<https://wikis.khronos.org/opengl/Blending>

Use this to distinguish color blending from visibility/depth decisions.

**[R12] Khronos transparency sorting guide**  
<https://wikis.khronos.org/opengl/Transparency_Sorting>

Read after you have reproduced the transparency problem yourself.

**[R13] Khronos polygon offset/depth-fighting background**  
<https://wikis.khronos.org/opengl/Basics_Of_Polygon_Offset>

This helps explain why coincident faces/lines can flicker or tear, even though
the project ultimately avoids duplicates through ownership rather than relying
on offsets.

**[R14] Git diff documentation**  
<https://git-scm.com/docs/git-diff>

Use path-limited comparisons so `baseline` supports reflection without
overwhelming the current checkpoint.

---

# Progress ledger

Update the status column as you work. Use `not started`, `working`, `review`, or
`complete`.

| Checkpoint | Short name | Status | Commit | Notes |
|---:|---|---|---|---|
| 1 | Software window | complete | | Build/review passed; software backend verified. |
| 2 | Visible frame change | complete | `0f6ceb6` | Build/review passed; movement is scaled by frame duration. |
| 3 | Diagnostic overlay | complete | `f52678c` | Build/review passed; fixed diagnostics render after animated content. |
| 4 | Runtime SDK | complete | `9b11d73` | Build/review passed; zero-state initialization and safe explicit shutdown. |
| 5 | 3D world | complete | | Build/review passed; fixed camera, grid, axes, and projection concepts. |
| 6 | One cube | complete | `bf72ba2` | Build/review passed; centered fill and contrasting wireframe. |
| 7 | Orbit derivation | complete | | Build/review passed; compact orbit state derives camera position. |
| 8 | Mouse orbit/zoom | complete | | Build/review passed; mouse orbit and clamped wheel zoom. |
| 9 | Cursor capture | complete | | Review passed; transition-only capture, focus recovery, and delta suppression. |
| 10 | Orbit SDK | complete | | Build/review passed; reusable orbit policy integrated with raw app input. |
| 11 | Tiny field | not started | | |
| 12 | Implicit centering | not started | | |
| 13 | Cube handles | not started | | |
| 14 | Cube values | not started | | |
| 15 | Deterministic generation | not started | | |
| 16 | Palettes and edges | not started | | |
| 17 | Proven-boundary refactor | not started | | |
| 18 | Selected cube | not started | | |
| 19 | Focused region | not started | | |
| 20 | Radius culling | not started | | |
| 21 | Input snapshot | not started | | |
| 22 | World navigation | not started | | |
| 23 | Camera navigation | not started | | |
| 24 | Vertical navigation | not started | | |
| 25 | Focused picking | not started | | |
| 26 | Direction metadata | not started | | |
| 27 | Font resources | not started | | |
| 28 | One face label | not started | | |
| 29 | Local face text | not started | | |
| 30 | Arrow primitive | not started | | |
| 31 | Six compass arrows | not started | | |
| 32 | Billboard labels | not started | | |
| 33 | Shaft gaps | not started | | |
| 34 | Boundary grids | not started | | |
| 35 | Immediate draw packets | not started | | |
| 36 | Render/frame data | not started | | |
| 37 | Allocator interface | not started | | |
| 38 | Persistent cube data | not started | | |
| 39 | Frame arena | not started | | |
| 40 | Memory requirements | not started | | |
| 41 | Observe transparency | not started | | |
| 42 | Six-face transparency | not started | | |
| 43 | Transparent ordering | not started | | |
| 44 | Birds-eye camera | not started | | |
| 45 | Small exterior shell | not started | | |
| 46 | Coarse sampling | not started | | |
| 47 | Representative geometry | not started | | |
| 48 | Birds-eye values/picking | not started | | |
| 49 | Outward faces/edges | not started | | |
| 50 | Final integration | not started | | |

## Starting prompt for Checkpoint 1

When you are ready, tell me:

> I am starting Checkpoint 1. Give me the concrete runnable target, constraints,
> useful hints, and relevant references. I will start coding immediately and ask
> questions as I encounter them; review and reflection come afterward.

I will coach that checkpoint only. We will move to Checkpoint 2 after you can
build, launch, review, reflect on, and commit the first software-rendered
window.

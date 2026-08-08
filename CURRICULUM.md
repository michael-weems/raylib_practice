# Software-Rendered Cube Field: Rebuild Curriculum

## Purpose

This curriculum guides you through rebuilding the application demonstrated by
the finished project under `reference/`. It is a training route, not a
transcription route. You will learn the project by making one small, visible
capability work at a time.

There are **50 checkpoints** in eight phases. Every checkpoint ends with an
executable you can launch and inspect. From Checkpoint 17 onward, every step is
about Raylib, graphics mathematics, rendering behavior, interaction, or measured
software-rendering performance. Generic refactoring and allocator design are
not curriculum subjects.

The destination is the same as the reference application, but your intermediate
code is allowed to be simpler. Do not design the final abstraction before you
have felt the problem it solves.

The learner is already a professional software developer. Explanations focus on
graphics APIs, coordinate spaces, event-driven interaction, rendering pipelines,
software-rasterization costs, and data/cache behavior—not general programming or
refactoring technique.

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
   code you just wrote to the underlying graphics, interaction, or performance
   concepts.
7. **Commit:** Preserve the working checkpoint before moving forward.

There is no up-front design gate. New behavior begins in the simplest working
location, including directly in `main.cpp`. A single source file with static
functions is an acceptable result for this training project.

When repeated graphics code, mismatched coordinate calculations, or unclear
resource lifetime creates a concrete problem, I may suggest one small helper,
struct, or module. That suggestion supports the graphics exercise; reorganizing
the program is never itself a checkpoint. You will develop architectural taste
later by building more graphics projects and noticing the boundaries repeatedly.

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

- `tools\build-src.ps1` succeeds with no new blocking warnings.
- The executable launches and displays the stated visible result.
- Closing the window exits normally.
- Previously completed controls still work.
- Review finds no graphics, interaction, lifetime, or performance issue that
  blocks later work.
- After implementation, you can explain the main data entering and leaving the
  new behavior.
- You have made a small checkpoint commit.

## Rules for using the finished reference

The runnable project under `reference/` is the definitive finished behavioral
and technical oracle. Its module layout is one possible implementation, not a
curriculum target. The `baseline` tag is older historical evidence. Neither is
starter code.

Before completing a checkpoint, you may freely inspect:

- `REPORT.md` for concepts;
- Raylib headers, examples, and vendored implementation notes;
- official documentation linked below;
- your own earlier commits.

After your checkpoint works, you may compare behavior or one graphics technique
with `reference/`. Its source organization is optional study material. To see
what exists without reading implementations:

```powershell
rg --files reference
```

Then inspect only the graphics behavior you are studying. Avoid opening the
reference implementation before attempting your own unless you are genuinely
blocked or intentionally studying alternatives.

Use `git show baseline:<path>` only when historical evidence is useful. Never
switch the working tree to `baseline` over uncommitted work.

## Final constraints to practice from the beginning

- C++11 with plain structs and free functions.
- Explicit for-loops; no iterator-style loops.
- No application classes, inheritance, exceptions, STL containers, smart
  pointers, templates, or lambdas.
- References for required borrowed inputs; pointers for real memory ranges,
  optional memory, callbacks, C strings, and pointer-plus-count streams.
- Straightforward initialization/shutdown for Raylib resources.
- Zero-based handles for stable array identities; valid handles occupy
  `[0, count)` and a zero-initialized handle identifies the first element.
- Compact contiguous hot data, X-contiguous traversal, and explicit reasoning
  about working sets, cache lines, L1/L2/L3, prefetch, and memory bandwidth.
- Immediate-mode rendering; no retained application scene in the SDK.
- No changes anywhere under `vendor/raylib`.

Helpers and modules are optional unless a graphics feature needs one shared
calculation to remain correct. Keep the visual result alive whenever code moves.

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

## Checkpoint 13 — Zero-based cube handles

**Challenge:** Convert between `(x,y,z)` and a zero-based 32-bit handle. Keep X
contiguous and distinguish the element count from the maximum valid handle.

**Visible finish:** The overlay shows the handle and recovered coordinate for a
visually highlighted cube; changing the highlighted coordinate preserves the
round trip.

**Reflect after:** How did you flatten the coordinates? Why is the valid handle
range `[0, count)` while the maximum valid handle is `count - 1`?

**Hints:** Derive the flattening formula with X as the fastest-changing axis,
then reverse it using division and remainder.

**Read:** `REPORT.md`, section 8.2, and [R6].

**Self-check:** Test the first and last handles, round-trip several interior
coordinates, and reject a handle equal to the element count.

## Checkpoint 14 — Cube values A through D

**Challenge:** Define byte-wide A/B/C/D categories and create one simple,
contiguous, application-owned value buffer with capacity `cube_count`. Associate
one category with every zero-based cube handle. This is provisional storage: do
not introduce the allocator interface or an `app` module yet.

**Visible finish:** The tiny field displays four clearly different fill colors,
and the overlay identifies the highlighted cube's value.

**Reflect after:** What changed when cubes began storing category bytes instead
of colors? Why does handle order now match the buffer's physical index order?

**Hints:** Create and initialize the buffer once before the frame loop. Use a
simple repeating pattern first, index it directly by a validated handle, and
release it explicitly at shutdown if your provisional storage requires that.
Keep position and identity implicit; only the category byte needs per-cube
storage. A temporary value-to-color switch is enough to make the data visible;
do not design the palette system early.

**Read:** `REPORT.md`, sections 8.2 and 8.3.

**Self-check:** Every handle in `[0, cube_count)` maps to exactly one initialized
byte and the last handle remains inside the allocation.

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
palettes. A `Cube_Visual_Style` keeps one value's fill and edge colors adjacent.
Palette handles are zero-based, and each palette contains adjacent styles for
A/B/C/D.

**Visible finish:** Keys 1/2/3 recolor every visible cube and its wireframe
without changing cube values. D is white at roughly 60% opacity in each palette.

**Reflect after:** Why did keeping fill and edge colors adjacent help? What
remained unchanged when the active palette changed?

**Hints:** Keep each palette's four styles contiguous and resolve a style from
the active palette handle plus the immutable cube value. Store semantics once
in the cube value buffer; do not cache a second per-cube color array.

**Read:** `REPORT.md`, section 8.4.

**Self-check:** Log or overlay one cube's value before and after palette changes;
the value must not change.

### Phase 3 gate

You should now be able to estimate the bytes required for 50 million categories,
explain why positions are not part of that estimate, and describe why the value
array is X-contiguous.

---

# Phase 4 — Focused view and navigation

This phase turns the field into an interactive inspection application while
teaching Raylib event semantics, camera bases, unprojection, and ray collision.

## Checkpoint 17 — Persistent selected target

**Challenge:** Turn the temporary highlighted coordinate into persistent selected
cube state and make the orbit camera target that cube's exact world-space center.
Add one temporary key that toggles between two known valid cubes so retargeting
can be observed before full navigation exists.

**Visible finish:** The camera orbits the highlighted cube rather than the world
origin. Toggling selection snaps the orbit center without resetting yaw, pitch,
or distance, and the overlay shows the selected handle/coordinate/value.

**Reflect after:** Which state is authoritative: handle, coordinate, world center,
or `Camera3D.target`? Which values are persistent and which are derived?

**Hints:** Derive the selected coordinate and center from the handle. Use the
same coordinate-to-center calculation for drawing and camera targeting so the
two systems cannot disagree.

**Read:** `REPORT.md`, sections 8.1–8.2 and 10.

**Self-check:** Orbit before and after retargeting. Orientation and zoom should
remain unchanged while the whole orbit sphere translates to the new target.

## Checkpoint 18 — A bounded focused region

**Challenge:** Render only a coordinate box extending five cubes from the
selected cube, clamped independently against all six field boundaries.

**Visible finish:** A small local neighborhood follows selection. Moving the
temporary selection between a center cube and an edge cube visibly clips the
rendered box without invalid memory access.

**Reflect after:** Why calculate candidate bounds once before the hot loops?
Where can signed/unsigned underflow appear near coordinate zero?

**Hints:** Keep the selected coordinate signed while subtracting the radius, or
explicitly clamp before converting to unsigned identities. Count candidates in
the overlay.

**Read:** `REPORT.md`, sections 12.2 and 13.

**Self-check:** Temporarily target each field corner and compare the candidate
count with the count at the center.

## Checkpoint 19 — Euclidean radius culling

**Challenge:** Keep the clamped candidate box, but accept only coordinates within
the current focused-radius sphere in grid space. The learner build currently
uses radius three; final radius tuning remains deferred.

**Visible finish:** The box corners disappear, leaving a rounded lattice volume
of 123 cubes away from field boundaries at radius three. The overlay reports
candidates tested and cubes submitted.

**Reflect after:** Why compare squared distance instead of distance? How does a
cheap CPU rejection reduce much more expensive software rasterization work?

**Hints:** Subtract coordinates in a signed type. Compare `dx*dx + dy*dy + dz*dz`
against `radius*radius` and avoid `sqrt` entirely.

**Read:** `REPORT.md`, sections 12.2 and 13.

**Self-check:** Predict accepted counts at the field center and at a corner,
then compare those predictions with the overlay.

## Checkpoint 20 — Raylib event semantics and discrete navigation

**Challenge:** Use Raylib keyboard queries to move selection one face-adjacent
cube in fixed world directions. Deliberately compare `IsKeyPressed()` with
`IsKeyDown()` before choosing the behavior appropriate for discrete snapping.

**Visible finish:** One key press produces exactly one neighbor transition and
one camera snap; holding the key does not advance hundreds of cubes per second.
Selection never leaves the field.

**Reflect after:** What is the difference between an input state, an edge event,
and a time-scaled continuous action? Which type describes orbiting, zooming, and
cube snapping?

**Hints:** Convert selected handle to coordinate, alter one axis, validate it,
then convert back to a handle. Keep the temporary toggle only until this works.

**Read:** The input section of [R1] and local Raylib core input examples.

**Self-check:** Tap, hold, and rapidly alternate keys at every relevant field
boundary.

## Checkpoint 21 — Visualize the camera basis

**Challenge:** Derive normalized forward, right, and up vectors from the current
camera and draw them as temporary colored lines at the selected cube.

**Visible finish:** Orbiting the camera rotates the basis visualization. Forward
points from camera toward target, right tracks screen-right, and up completes a
consistent orthogonal frame.

**Reflect after:** Why is `camera.up` not always identical to the derived view-up?
How do normalization, dot products, and cross products establish a basis?

**Hints:** Start with `target - position`. Use cross products in a deliberate
order and verify the result visually; swapping operands flips the direction.

**Read:** [R5], `raymath.h`, and `3D_SPACE_CURRICULUM.md` vector-basis exercises.

**Self-check:** Test near several yaw angles and near both pitch limits. The
three vectors should remain perpendicular and should not suddenly mirror.

## Checkpoint 22 — Camera-relative horizontal navigation

**Challenge:** Make left/right/forward/back choose grid neighbors relative to
what appears on screen rather than fixed world X/Z directions.

**Visible finish:** After orbiting around the selected cube, navigation still
moves visually left, right, away, or toward the viewer on the horizontal grid.

**Reflect after:** How do continuous camera vectors become one discrete grid-axis
step? What should happen near a 45-degree tie?

**Hints:** Flatten forward and right onto the XZ plane. Compare absolute X and Z
components, choose the dominant axis, and keep its sign.

**Read:** [R5] and `REPORT.md`, section 10.

**Self-check:** Test near 0°, 45°, 90°, 135°, and 180° yaw and describe the tie
policy you selected.

## Checkpoint 23 — Pitch-aware vertical navigation

**Challenge:** When the camera looks steeply upward or downward, allow the same
forward/back intent to traverse cube layers along field Y.

**Visible finish:** Shallow pitch navigates the XZ plane. Steep pitch moves
between Y layers in the direction that feels visually forward/back.

**Reflect after:** What camera-space signal indicates vertical intent? Why is a
threshold or dominant-component decision necessary?

**Hints:** Use the derived camera forward vector, not mouse delta or raw pitch
input. Compare its absolute Y component against its horizontal components.

**Read:** `REPORT.md`, sections 5 and 10.

**Self-check:** Test positive and negative steep pitch at both the top and bottom
field boundaries.

## Checkpoint 24 — Screen-to-world ray visualization

**Challenge:** While the cursor is free, call Raylib's screen-to-world ray API
and draw the resulting ray into the scene without selecting anything yet.

**Visible finish:** A line begins at the camera and passes through the mouse's
screen position into the 3D scene. Moving the cursor changes its direction while
the camera remains still.

**Reflect after:** How does one 2D pixel describe an infinite 3D ray rather than a
single world point? Which camera projection data is required to unproject it?

**Hints:** Use `GetScreenToWorldRay()` or the exact equivalent exposed by the
vendored header. Draw only a finite segment for inspection.

**Read:** [R3] and local `vendor/raylib/examples/core/core_3d_picking.c`.

**Self-check:** Point at the selected cube's center and at empty background;
explain why both produce valid rays.

## Checkpoint 25 — Nearest-hit focused picking

**Challenge:** Test the click ray against axis-aligned cube bounding boxes in the
same focused neighborhood used for drawing, then select the nearest hit.

**Visible finish:** Right-click releases the cursor; left-clicking a visible cube
snaps the target to exactly that cube and recaptures the cursor.

**Reflect after:** Why must all candidates be tested before selecting? What does
collision distance solve when boxes overlap on screen?

**Hints:** `GetRayCollisionBox()` reports both `hit` and `distance`. Test every
candidate before committing selection; a later candidate can be closer. On a
successful click, recapture the cursor and suppress the first mouse delta.

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

## Checkpoint 26 — Face normals and local coordinate frames

**Challenge:** Define the six cube faces using outward normal, face-local right
and down vectors, axis identity, and short label. Draw each normal and a small
right/down cross on the selected cube.

**Visible finish:** The selected cube draws six small colored normal lines or
face markers, each labeled in the 2D overlay.

**Reflect after:** How do a normal and two tangent vectors define a 2D coordinate
system embedded in 3D? Which cross-product order gives the expected handedness?

**Hints:** Verify one face at a time. Stand mentally outside the face and check
whether local right/down would produce readable rather than mirrored text.

**Read:** `REPORT.md`, sections 12.5–12.7.

**Self-check:** From outside each face, local right/down should orient text
consistently rather than mirror it.

## Checkpoint 27 — Persistent font resources

**Challenge:** Load the supplied Fira Code font once before the frame loop, use it
for the existing overlay, and unload it after the loop.

**Visible finish:** Replace the default overlay font with the loaded font and
show an ownership/resource-ready diagnostic.

**Reflect after:** Which parts of a `Font` live in CPU memory and which become a
texture consumed by rendering? Why would loading or rasterizing it every frame
be wasteful?

**Hints:** Study `LoadFontEx()`, `IsFontValid()`, `DrawTextEx()`, and
`UnloadFont()`. A local variable in `main.cpp` is completely acceptable.

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
distant local cubes and back-facing faces do not. Diagnostics report visible
labeled faces and glyphs submitted for the frame.

**Reflect after:** Which text widths repeat and can be measured once? How does a
face normal dot camera direction identify a back face?

**Hints:** Submit fills before text. Keep the dynamic handle formatting local to
the draw loop without allocating strings on the heap. Measure a
`RelWithDebInfo` local-view frame with text enabled and disabled so you can
separate text cost from cube geometry cost.

**Read:** `REPORT.md`, sections 12.2 and 12.6.

**Self-check:** Verify labeled-face and glyph counts drop when looking from
another side; record the optimized-build timing difference with text disabled.

## Checkpoint 30 — One compass arrow from 3D primitives

**Challenge:** Draw one +X arrow from a cylinder shaft and cone arrowhead using
Raylib's 3D primitive functions.

**Visible finish:** A clearly proportioned +X arrow extends from the selected
cube.

**Reflect after:** How do two endpoints define direction, length, and midpoint?
Which calculations require a normalized direction and which require real length?

**Hints:** Raylib's cylinder-between-points operation can represent both a
cylinder and a cone. Start directly in the draw loop; extract a helper only when
drawing the other five arrows would duplicate the same geometry math.

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

**Hints:** Reuse the same arrow math with different start/end points and colors.
No module boundary is required.

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

# Phase 6 — The software graphics pipeline and CPU performance

This phase turns familiar draw calls into observable pipeline behavior. It also
connects cache locality and working-set size to the much larger cost of CPU
rasterization.

## Checkpoint 35 — Prove the renderer is immediate-mode

**Challenge:** Add controls that omit cube fills, edges, text, and annotations
from individual frames while leaving their source data untouched. Also make one
known cube blink by intentionally skipping its draw call on alternating periods.

**Visible finish:** Any omitted feature disappears immediately and returns when
submitted again; Raylib retains no application cube scene between frames. The
overlay reports submission counts for each enabled feature.

**Reflect after:** What persists across frames: cube values, camera state, font
resources, or draw submissions? Why can a transient command list still belong to
an immediate-mode renderer?

**Hints:** Toggle branches around existing draw calls. Do not create packets or a
renderer abstraction merely to demonstrate the concept.

**Read:** `REPORT.md`, sections 3.1, 7, and 12.1.

**Self-check:** Pause camera input and skip a draw for one frame; nothing from the
previous frame should remain after `ClearBackground()`.

## Checkpoint 36 — Explicit faces, winding, and back-face culling

**Challenge:** Replace one diagnostic cube with six explicit quad faces, then
toggle face culling and deliberately reverse one face's vertex order.

**Visible finish:** Correctly wound outward faces remain visible from outside.
The reversed face disappears or behaves oppositely when culling is enabled, and
the overlay identifies which mode is active.

**Reflect after:** How does vertex order define a front face? How are geometric
normal, winding, and the face-local basis related but not identical?

**Hints:** Use `rlBegin(RL_QUADS)` or triangles through `rlgl` in project-owned
code. Change one variable at a time: first explicit geometry, then culling, then
winding.

**Read:** Raylib's `rlgl.h`, `REPORT.md` sections 3.3–3.5, and [R13] for later
depth-fighting context.

**Self-check:** Inspect all six faces from outside and inside with culling both
enabled and disabled.

## Checkpoint 37 — Depth testing versus submission order

**Challenge:** Draw two overlapping opaque cubes with strongly different colors,
toggle depth testing, and reverse their submission order.

**Visible finish:** With depth testing enabled, the nearest surfaces win
regardless of submission order. With it disabled, later submitted fragments win
where geometry overlaps.

**Reflect after:** What information does the depth buffer store per pixel? What is
the difference between testing depth and writing depth?

**Hints:** Use fixed test geometry before applying conclusions to the full field.
The experiment is the feature; remove or disable it after recording the result.

**Read:** [R10] and `REPORT.md`, sections 3.6 and 12.4.

**Self-check:** Predict the resulting visible color for both draw orders and both
depth modes before toggling them.

## Checkpoint 38 — Projection and clipping planes

**Challenge:** Add a diagnostic mode that switches between perspective and
orthographic projection and allows controlled near/far clip-plane changes while
drawing known objects at measured distances.

**Visible finish:** Perspective makes distant objects appear smaller;
orthographic projection preserves apparent size. Tight near/far planes visibly
slice geometry, and the overlay reports projection and clip distances.

**Reflect after:** How are camera space, clip space, perspective division, and the
depth range connected? Why can a distant birds-eye camera require a very
different far plane?

**Hints:** Reuse the existing camera and use `rlSetClipPlanes()` only in
project-owned code. Restore normal clip values after the experiment each frame.

**Read:** `rlgl.h`, [R10], and `REPORT.md`, sections 3.3 and 3.8.

**Self-check:** Place one object before the near plane, one inside the frustum,
and one beyond the far plane, then explain which vertices survive clipping.

## Checkpoint 39 — Cache lines and traversal locality

**Challenge:** Increase only the semantic value buffer to the final
`500 × 1000 × 100` logical dimensions while continuing to draw the bounded local
region. Add an on-demand CPU experiment that reads the same values once in
X-contiguous order and once in a deliberately strided order, producing identical
checksums and reporting elapsed time.

**Visible finish:** The scene remains responsive because only the focused region
is submitted. A key-triggered diagnostic reports roughly 50 million value bytes,
the working-set size in MiB, and timings for sequential versus strided traversal.

**Reflect after:** How many one-byte values fit in a typical 64-byte cache line?
What roles do L1, L2, L3, hardware prefetch, and main-memory bandwidth play?
Why does loop order matter when X is the contiguous dimension?

**Hints:** Run the experiment outside drawing and only on request; do not scan 50
million cubes every frame. Accumulate into a checksum so the compiler cannot
delete the reads. Repeat enough times to distinguish noise, then report medians
or the clearest stable observation rather than claiming a universal benchmark.

**Read:** [R9] and `REPORT.md`, sections 8.5 and 13.

**Self-check:** Both traversals must produce the same checksum. Record build type,
machine, traversal order, and working-set size with the timing.

## Checkpoint 40 — Measure software-rasterization cost

**Challenge:** In a clean `RelWithDebInfo` build, measure focused-view performance
with fills, edges, face text, compasses, and grids toggled independently. Report
candidate cubes, submitted cubes/faces/lines/glyphs, and frame time or FPS.

**Visible finish:** The overlay makes it clear which visible feature consumes
time. The normal focused configuration remains at least 30 FPS, targeting 60,
without scanning the entire logical field.

**Reflect after:** Which costs came from cache/memory access, CPU geometry setup,
glyph submission, overdraw, and pixel rasterization? Why is drawing less usually
more valuable than shaving arithmetic from a tiny loop?

**Hints:** Change one feature at a time and keep camera position fixed while
comparing. Use `tools\build-performance.ps1 -App src -BuildType RelWithDebInfo`;
Debug FPS is not evidence.

**Read:** `REPORT.md`, sections 3.5, 8.5, 13, and 14.

**Self-check:** Record a small table of feature combinations and measurements.
Keep optimizations only when the counters and timings support the explanation.

### Phase 6 gate

Before transparency, explain one frame from semantic value load through style
lookup, vertex submission, clipping, rasterization, depth testing, and final
pixel color. Also explain why the bounded X-contiguous local loop has a much
smaller working set than the 50-million-byte source buffer.

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
keep the fix in project-owned code. A small face-emission helper is useful if it
keeps all six faces consistent, but no module extraction is required.

**Read:** `REPORT.md`, sections 12.4 and 14, and local
`vendor/raylib/src/external/rlsw.h` as a diagnostic reference.

**Self-check:** Verify all face directions and multiple D cubes.

## Checkpoint 43 — Back-to-front transparent commands

**Challenge:** Collect transparent face commands into one contiguous
fixed-capacity scratch array, assign camera depth, sort far-to-near, and submit
after opaque fills with depth writes disabled.

**Visible finish:** Overlapping transparent cubes blend consistently while
opaque cubes still occlude them correctly. Diagnostics report opaque faces,
transparent faces, command bytes, and sort work.

**Reflect after:** Why is the command array still immediate-mode even if its
backing allocation persists? Why sort faces rather than cube centers? Why retain
depth testing but disable depth writes? How does contiguous command storage help
cache traversal during sorting and submission?

**Hints:** Regenerate commands and reset the count every frame. The bounded local
radius gives a calculable maximum face count, so a caller-owned scratch block or
fixed array is enough; an arena allocator is not part of this course. Use an
in-place sort and distinguish collection/sorting cost from rasterization cost.

**Read:** [R10], [R11], [R12], and `REPORT.md`, section 12.4.

**Self-check:** Render a controlled near/far pair whose expected color layering
you can explain, then verify command count never exceeds the capacity derived
from the maximum focused workload.

### Phase 7 gate

You should now be able to explain color-buffer blending separately from
depth-buffer testing and writing. This distinction is foundational.

---

# Phase 8 — Birds-eye mode

This phase introduces a second representation of the same source data.

## Checkpoint 44 — A second camera mode

**Challenge:** Add birds-eye mode toggled by `G`, with its own orbit/zoom preset,
a target at field center, and clip planes derived for the field's scale.

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

## Checkpoint 50 — Field-scale compass and final graphics integration

**Challenge:** Add the large birds-eye compass and its readable billboard labels,
complete the final `500 × 1000 × 100` focused/birds-eye experience, and validate
software-rendering performance in a clean `RelWithDebInfo` build.

**Visible finish:** The complete application matches the reference behavior:
focused and birds-eye views, navigation, picking, palettes, transparency, text,
compasses, boundary grids, and 30+ FPS software rendering.

**Reflect after:** Which visual features scale with selected cube, virtual shell,
camera distance, or screen resolution? Which coordinate-space or pipeline
misunderstandings caused the hardest bugs? Where is actual frame time spent?

**Hints:** Center one compass on each outer field face, scale it from field
extents, use white billboard text, and suppress labels when their anchor belongs
behind the field. Profile before changing algorithms. Report focused and
birds-eye frame times alongside candidate, submitted, face, glyph, and
transparent-command counters so a result can be reproduced.

**Read:** `REPORT.md`, sections 13–20, [R9], and [R14].

**Self-check:** Run all controls and the isolated optimized performance build.
Verify 30+ FPS in both modes, record the machine/build/backend and counters used
for the claim, and compare visible behavior with `reference/`. Source structure
may differ completely.

### Final gate

You are finished when you can rebuild the graphics behavior from your own
reasoning, not when your source or architecture resembles `reference/`.

Prepare a short retrospective answering:

1. Which performance wins came from drawing less rather than drawing faster?
2. Which bugs were really coordinate-space misunderstandings?
3. Which bugs came from depth, blending, winding, clipping, or draw ordering?
4. How did memory layout and bounded working sets affect CPU behavior?
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
- `reference/` — definitive finished implementation for post-attempt study.
- Git tag `baseline` — historical design evidence only.

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

Use path-limited comparisons so `reference/` supports reflection without
overwhelming the current checkpoint. Use `git show` only when older `baseline`
history answers a specific design question.

---

# Progress ledger

This ledger is the authoritative progress record. Update it only after review,
using `not started`, `working`, `review`, or `complete`. `CURRENT_STEP.md`
contains the detailed brief for the first incomplete checkpoint; `CONTEXT.md`
links here instead of duplicating checkpoint history.

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
| 11 | Tiny field | complete | | Build/review passed; field renders from explicit nested loops without storing positions. |
| 12 | Implicit centering | complete | | Build/review passed; arbitrary even/odd dimensions stay centered around world origin. |
| 13 | Cube handles | complete | | Build/review passed; zero-based X-contiguous handles round-trip through grid coordinates. |
| 14 | Cube values | complete | | Build/review passed; one-byte immutable A/B/C/D values use compact zero-based handles. |
| 15 | Deterministic generation | complete | | Build/review passed; startup coordinate hashing produces immutable reproducible A/B/C/D values without storage-order striping. |
| 16 | Palettes and edges | complete | | Build/review passed; three zero-based palettes map immutable values to adjacent fill/edge styles and switch immediately with keys 1/2/3. |
| 17 | Selected target | complete | | Build/review passed; persistent zero-based selection drives the orbit target without resetting orientation or zoom. |
| 18 | Focused bounds | complete | | Build/review passed; radius-three box directly enumerates 343 interior or 64 corner candidates with X-contiguous traversal. |
| 19 | Radius culling | working | | Reject bounded-box coordinates outside the selected cube's Euclidean grid-space radius. |
| 20 | Discrete navigation events | not started | | |
| 21 | Camera basis visualization | not started | | |
| 22 | Camera-relative navigation | not started | | |
| 23 | Vertical navigation | not started | | |
| 24 | Screen-to-world ray | not started | | |
| 25 | Focused picking | not started | | |
| 26 | Face coordinate frames | not started | | |
| 27 | Font resources | not started | | |
| 28 | One face label | not started | | |
| 29 | Local face text | not started | | |
| 30 | Arrow geometry | not started | | |
| 31 | Six compass arrows | not started | | |
| 32 | Billboard labels | not started | | |
| 33 | Shaft gaps | not started | | |
| 34 | Boundary grids | not started | | |
| 35 | Immediate-mode experiment | not started | | |
| 36 | Winding and culling | not started | | |
| 37 | Depth testing | not started | | |
| 38 | Projection and clipping | not started | | |
| 39 | Cache locality | not started | | |
| 40 | Rasterization measurement | not started | | |
| 41 | Observe transparency | not started | | |
| 42 | Six-face transparency | not started | | |
| 43 | Transparent ordering | not started | | |
| 44 | Birds-eye camera | not started | | |
| 45 | Small exterior shell | not started | | |
| 46 | Coarse sampling | not started | | |
| 47 | Representative geometry | not started | | |
| 48 | Birds-eye values/picking | not started | | |
| 49 | Outward faces/edges | not started | | |
| 50 | Final graphics integration | not started | | |

## Resuming the curriculum

Open `CURRENT_STEP.md` for the active build brief. Implement that checkpoint in
the learner project under `src/`, ask for graduated hints when blocked, and
submit the running result for review. The coach updates this ledger only after
the checkpoint passes.

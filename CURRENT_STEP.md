# Current Training Step

## Resume here

Checkpoints 1 through 21 are complete. Checkpoint 22 is **Camera-relative
horizontal navigation**.

The camera basis from Checkpoint 21 is now trusted. This checkpoint will use
that continuous orientation to choose a discrete neighboring cube. Keep the
basis visualization while developing; it is useful evidence that the selected
grid step agrees with the camera.

## Runnable goal

Replace fixed-world X/Z selection controls with camera-relative controls:

- `H` selects the cube that appears to the left;
- `L` selects the cube that appears to the right;
- `U` selects the cube past the target, away from the viewer;
- `I` selects the cube before the target, toward the viewer.

Each press must still select exactly one face-adjacent cube. `J` and `K` may
remain fixed world-Y controls for this checkpoint. Checkpoint 23 will decide
when forward/back intent should move through Y layers.

## New graphics concept: quantizing a continuous direction

The camera's basis vectors contain floating-point directions. At most yaw
angles, camera-forward and camera-right do not align exactly with world X or Z:

```text
camera-right = ( 0.81, 0.00,  0.59 )
```

Moving by both components would produce a diagonal grid step. That would skip
the face-neighbor rule and could change two coordinates from one key press.

Instead, quantize the continuous direction onto the nearest horizontal world
axis:

```text
continuous camera direction
       -> remove Y
       -> compare |X| with |Z|
       -> retain the dominant axis and its sign
       -> one of (-X, +X, -Z, +Z)
```

For the example above, `|0.81| > |0.59|`, so screen-right becomes one `+X`
grid step.

This is a small form of vector classification. Four regions of the horizontal
direction circle map to four cardinal grid directions.

## Flattening onto the horizontal plane

Checkpoint 22 deliberately ignores vertical camera intent. Project a camera
direction onto the world XZ plane by discarding its Y component:

```text
horizontal_forward = (forward.x, 0, forward.z)
horizontal_right   = (right.x,   0, right.z)
```

Normalizing the flattened vector can make its meaning clearer, but it does not
change which of X or Z is dominant. Do not use the discarded Y component in
the decision yet.

The current pitch clamp keeps horizontal-forward from becoming exactly zero.
Checkpoint 23 will revisit steep pitch instead of adding fallback behavior now.

## Turning semantic intent into a direction

Treat a key as an intent before treating it as a coordinate mutation:

```text
screen-right     -> horizontal camera-right
screen-left      -> negative horizontal camera-right
away from viewer -> horizontal camera-forward
toward viewer    -> negative horizontal camera-forward
```

Remember that `forward = target - position`. Extending forward through the
target therefore travels farther away from the viewer. Negating it travels
back toward the camera.

Once an intended direction has been selected, compare the magnitudes of its X
and Z components. Change only the dominant coordinate by `+1` or `-1` according
to that component's sign.

Use signed temporary deltas even though stored grid coordinates are unsigned.
Apply the delta and clamp against the field boundary before converting the
coordinate back into a handle.

## The 45-degree boundary

At exactly a diagonal direction, `|X| == |Z|`. There is no uniquely nearest
cardinal axis, so the program needs a deterministic tie policy.

For example, an `>=` comparison can make X win ties; changing the comparison
can make Z win. Either policy is acceptable if it is deliberate and consistent.
Near the boundary, a tiny yaw change will switch sectors. That is expected:
the grid offers no diagonal result in this checkpoint.

Do not add smoothing, accumulated movement, or a stored navigation direction.
One key-edge event produces one classification and one step.

## Frame-order challenge

The existing fixed-world switch runs before this frame's `Camera3D` and basis
exist. Camera-relative navigation cannot make its decision there without using
stale or independently reconstructed camera math.

Reorganize the frame so that:

1. Raylib input is sampled once.
2. The current orbit orientation produces a `Camera3D` and basis.
3. A navigation event is classified using that basis.
4. A successful step changes the selected coordinate, handle, and world target.
5. The final camera and rendered neighborhood agree on the same selected cube.

Do not process the same mouse delta or wheel delta through
`orbit_camera_update()` twice. That would rotate or zoom twice in one frame and
could also repeat cursor-transition side effects.

When a target snaps but yaw, pitch, and distance do not change, this invariant
should remain true:

```text
camera.position - camera.target = unchanged orbit offset
```

That invariant is the clue for keeping the final current-frame `Camera3D`
coherent after selection changes. Work out the simplest ordering or translation
that preserves it. If this becomes the blocking part, ask for a more specific
hint before changing the orbit-camera API.

## Build brief

1. Continue deriving forward and right from the completed current camera.
2. Flatten the relevant directions onto XZ.
3. Convert the four navigation keys into semantic intended directions.
4. Quantize an intended direction onto exactly one signed X/Z grid step.
5. Clamp the resulting coordinate to the field.
6. Recalculate the selected handle and target using existing helpers.
7. Keep the camera orbit offset coherent during the snap.
8. Keep the basis visualization and add concise navigation diagnostics if they
   help verify the chosen world-axis step.

This may remain directly in `main.cpp`. Do not create a navigation module or a
general vector-quantization abstraction for one use site.

## Immediate-mode and memory intent

All navigation math is transient and derived from this frame's camera plus one
input event:

```text
Camera3D + pressed key -> intended direction -> signed grid delta -> selection
```

No navigation array, lookup table, heap allocation, retained command, or cube
scan is needed. The work is a handful of scalar comparisons performed only
when a relevant key is pressed. It does not touch the 50-million-value cube
array until normal rendering reads the small visible neighborhood.

## Visible finish

- `H` always moves visually left after orbiting.
- `L` always moves visually right after orbiting.
- `U` moves to the horizontal cube beyond the selected cube from the viewer.
- `I` moves horizontally toward the viewer.
- Every press changes zero coordinates at a boundary or exactly one coordinate
  elsewhere.
- Movement remains one discrete step per press.
- Snapping does not reset yaw, pitch, distance, or cursor capture.
- The camera, highlight, basis origin, culling neighborhood, and overlay all
  agree on the newly selected cube in the rendered frame.
- Palette switching and existing J/K behavior still work.

## Self-checks

Orbit near these yaw angles and test all four camera-relative controls:

```text
0 degrees
just below / exactly / just above 45 degrees
90 degrees
135 degrees
180 degrees
270 degrees
```

Also test:

- all four X/Z field boundaries;
- shallow and steep positive/negative pitch;
- alternating opposite keys rapidly;
- orbiting immediately before a navigation press;
- holding a key rather than tapping it;
- palette keys after the input reordering.

The camera basis lines should let you predict the chosen X/Z axis before each
press. If screen-left becomes screen-right, inspect vector negation. If X and Z
are swapped at unexpected angles, inspect the dominant-axis comparison. If the
camera jumps independently of the selected cube, inspect the frame-order orbit
offset invariant.

## Constraints

- Use the current camera basis, not raw yaw or duplicated trigonometry.
- Ignore Y when classifying horizontal navigation in this checkpoint.
- Produce only face-adjacent grid movement; no diagonal steps.
- Keep the existing discrete Raylib key-event behavior.
- Choose and understand a deterministic 45-degree tie policy.
- Do not scan cube data to navigate.
- Do not allocate or retain derived navigation data.
- Do not change the orbit-camera API unless the current attempt proves it is
  necessary.
- Do not implement pitch-aware Y selection yet.

## References if blocked

- `raymath.h`: `Vector3Negate()`, `Vector3Normalize()`, and vector components.
- `REPORT.md`, section 10, for the finished application's navigation concept.
- `3D_SPACE_CURRICULUM.md` for projection and basis exercises.
- Raylib keyboard input documentation for edge-triggered key events.

Do not copy the finished navigation implementation from `reference/`; use it
only after your own attempt if runtime behavior remains unexplained.

## Review target

Submit the running implementation when ready. Review will check:

- current camera forward/right drive semantic navigation;
- vertical components are excluded from horizontal classification;
- dominant-axis and sign selection are correct and deterministic;
- one press produces at most one face-neighbor step;
- unsigned coordinates cannot underflow or overflow;
- camera/selection/render state is coherent in the same frame;
- no mouse/wheel input or cursor transition is accidentally processed twice;
- no allocations, field scans, or retained render state were introduced;
- the learner build passes.

Reflection follows after it works: why is dominant-axis classification a form
of quantization, what information is discarded, and why can a deterministic
answer near 45 degrees still change abruptly?

## Resume prompt

> I am starting Checkpoint 22 from `CURRENT_STEP.md`: flatten the current
> camera basis onto XZ, quantize left/right/forward/back intent into one signed
> face-neighbor step, and keep the snapped camera target coherent without
> processing frame input twice.

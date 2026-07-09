# Raylib Mathematics and 3D-Space Practice Curriculum

## Purpose

This is a separate companion track to `CURRICULUM.md`. It develops the
geometry, trigonometry, and linear-algebra intuition needed for 2D/3D graphics,
camera work, animation, picking, and transformations without interrupting the
main software-renderer project.

The assumed starting point is a working Raylib application with a frame loop.
Each session should fit in about 30 minutes and end with a visible, runnable
result. The goal is not to memorize formulas or solve the design on paper first.
The goal is to build a small experiment, observe it, perturb it, and explain the
mathematics afterward.

## Schedule

- Duration: 12 weeks
- Practice days: 5 per week
- Session length: 30 minutes
- Total sessions: 60
- Weekend: optional review, catch-up, or repetition without adding new ideas

Do not complete several sessions in one sitting just because they are short.
Spacing and recall are part of the training.

## The 30-minute routine

1. **Launch — 2 minutes.** Read the visible build target and start in the
   working Raylib project. Do not wait for a complete paper design.
2. **Build — 18 minutes.** Implement the smallest visual experiment that can
   expose the day's concept.
3. **Debug and perturb — 7 minutes.** Ask for a hint if blocked; otherwise
   change signs, magnitudes, or one axis at a time and observe the result.
4. **Reflect — 3 minutes.** After it runs, write two sentences explaining what
   happened and why.

The coaching loop is build-first: I provide the day's target, constraints,
hints, and references; you code; I answer questions just in time; then I review
the result and ask reflection questions. Prediction is a useful experiment
during perturbation, not an entrance exam before coding.

If stuck, use this hint ladder:

1. Draw the problem on paper and label known values.
2. Test a boundary case such as zero, one, 90 degrees, or equal points.
3. Print or draw intermediate values on screen.
4. Consult the formula card near the end of this document.
5. Read the smallest relevant local Raylib example.
6. Ask for one hint before asking for finished code.

## Practice rules

- Keep each exercise runnable at the end of the session.
- Use radians internally. Convert degrees only at user-facing boundaries.
- Multiply rates by `GetFrameTime()`.
- Draw axes, points, vectors, normals, and intermediate constructions.
- Change one variable at a time, observe the effect, and explain it afterward.
- Use plain structs, free functions, and explicit loops.
- Avoid hidden state and avoid storing both authoritative and derived values.
- Implement important math once yourself, then compare with `raymath.h`.
- Keep each new experiment local until repetition or a clear reusable boundary
  gives you a concrete reason to extract it.
- Commit or save one small source snapshot per completed week.

## Coordinate conventions used here

### Raylib 2D

- Origin is normally the upper-left of the screen.
- +X points right.
- +Y points down.
- Clockwise/counterclockwise appearance can therefore surprise you.

### Project 3D convention

- +Y is world up.
- The horizontal ground plane is XZ.
- Project axis colors are X blue, Y red, and Z green.
- A camera target is a world-space point, not an angle.
- Yaw rotates around the Y axis.
- Orbit pitch is elevation above the XZ plane.
- Roll rotates the camera around its viewing direction.

## Important terminology

- A **point** is a location. A **vector** is a displacement or direction.
- **Magnitude** is vector length.
- A **unit vector** has magnitude one.
- **Linear transforms** preserve vector addition/scaling and keep the origin
  fixed. Rotation about the origin, scale, and shear are examples.
- **Translation is affine**, not strictly linear, because it moves the origin.
- A **nonlinear transform** can bend or warp space; sine waves and radial
  twists are examples.
- A **basis** is a set of directions used to describe coordinates.
- A **vertex** is a geometric point submitted as part of a primitive.
- A **normal** is a direction perpendicular to a surface.

---

# Week 1 — Points, vectors, and motion in 2D

## Day 1 — See the 2D coordinate system

- **Build:** Draw an origin marker, +X/+Y arrows, mouse position, and coordinate
  text. Let a key move the origin between screen center and upper-left.
- **Math:** Coordinates are measured relative to an origin and basis.
- **Perturb:** Predict where `(100, 50)` appears under both origins.
- **Done when:** You can explain why increasing Y moves downward in screen
  space.

## Day 2 — Point versus displacement

- **Build:** Draw points A and B, then draw the arrow `B - A` beginning at A.
  Move B with the mouse.
- **Math:** Subtracting points produces a displacement; adding that displacement
  back to A reconstructs B.
- **Perturb:** Reverse the subtraction and identify what changes.
- **Done when:** The arrow always begins at A and terminates at B.

## Day 3 — Distance and magnitude

- **Build:** Draw a circle centered at A whose radius equals the distance from
  A to the mouse. Display `dx`, `dy`, and length.
- **Math:** `length = sqrt(dx*dx + dy*dy)`.
- **Perturb:** Hold either dx or dy at zero; test a 3-4-5 triangle.
- **Done when:** You can predict the radius from a few integer offsets.

## Day 4 — Normalize a direction

- **Build:** Draw one arrow from A to the mouse and a second arrow with constant
  length 100 pointing in the same direction.
- **Math:** Normalize by dividing each component by the magnitude.
- **Perturb:** Move the mouse onto A and decide how zero length should behave.
- **Done when:** The normalized arrow never changes length and zero is handled
  deliberately.

## Day 5 — Velocity in units per second

- **Build:** Move an arrowhead across the screen from a position and velocity.
  The arrow itself visualizes velocity.
- **Math:** `position += velocity * elapsed_seconds`.
- **Perturb:** Compare 30 and 60 target FPS, then double only velocity.
- **Done when:** Real-world motion speed is frame-rate independent.

### Week 1 recall

Without looking anything up, define point, vector, displacement, magnitude,
unit vector, velocity, and delta time. Rebuild Day 4 from a blank file if any
definition feels fuzzy.

---

# Week 2 — Angles and trigonometry in 2D

## Day 6 — Radians and the unit circle

- **Build:** Animate a point around a circle using one angle. Draw its X and Y
  component projections to the axes.
- **Math:** One full turn is `2*pi` radians. Cosine and sine are unit-circle
  coordinates.
- **Perturb:** Pause at 0, pi/2, pi, and 3pi/2 and predict the signs.
- **Done when:** You can identify the quadrant from sine/cosine signs.

## Day 7 — Aim with `atan2`

- **Build:** Draw an arrow at screen center that aims at the mouse. Display its
  angle in radians and degrees.
- **Math:** `atan2(dy, dx)` preserves the quadrant that ordinary division loses.
- **Perturb:** Visit all four quadrants and both axes.
- **Done when:** The arrow never flips into the wrong quadrant.

## Day 8 — Rotate a vector

- **Build:** Rotate a fixed arrow around its origin over time without changing
  its length. Draw the original arrow faintly.
- **Math:** Derive the 2D rotation from sine/cosine rather than using a sprite
  rotation API.
- **Perturb:** Test 0, 90, 180, and -90 degrees.
- **Done when:** Length is invariant within floating-point tolerance.

## Day 9 — Perpendicular vectors and a local basis

- **Build:** From a normalized forward arrow, construct and draw its left/right
  perpendicular. Use the pair to draw a small arrowhead or oriented rectangle.
- **Math:** In 2D, swapping components and negating one produces a perpendicular.
- **Perturb:** Reverse which component is negated and observe handedness.
- **Done when:** Forward dot right is approximately zero.

## Day 10 — Dot product as alignment

- **Build:** Draw two unit arrows from one origin and color their angle green,
  yellow, or red based on their dot product.
- **Math:** For unit vectors, dot equals cosine of the angle between them.
- **Perturb:** Test same, opposite, and perpendicular directions.
- **Done when:** You can predict dot values `1`, `0`, and `-1` visually.

### Week 2 recall

Sketch a unit circle and label sine/cosine at the four cardinal angles. Explain
why `atan2` accepts two components and why the dot product is useful without
calling `acos`.

---

# Week 3 — Projection, rejection, reflection, and steering

## Day 11 — Project one vector onto another

- **Build:** Draw vector V, basis direction B, and V's projection onto B. Let
  the mouse control V.
- **Math:** Projection measures how much of V lies along B.
- **Perturb:** Use normalized and non-normalized B and observe why the formulas
  differ.
- **Done when:** Projection is zero when V is perpendicular to B.

## Day 12 — Vector rejection

- **Build:** Add the component of V perpendicular to B. Draw projection and
  rejection head-to-tail so they reconstruct V.
- **Math:** `rejection = V - projection`.
- **Perturb:** Drag V across both sides of B.
- **Done when:** Projection plus rejection always reaches V's endpoint.

## Day 13 — Reflect a moving vector

- **Build:** Move a ball toward a wall line and reflect its velocity at impact.
  Draw the wall normal, incoming velocity, and reflected velocity.
- **Math:** Reflection removes twice the component pointing into the normal.
- **Perturb:** Try shallow, perpendicular, and reversed normal directions.
- **Done when:** Angle-in visually matches angle-out.

## Day 14 — Closest point on a segment

- **Build:** Draw a segment and the closest point on it to the mouse. Draw a
  line from the mouse to that point.
- **Math:** Project onto the segment direction, then clamp the parameter to
  `[0,1]`.
- **Perturb:** Move before A, between A/B, and beyond B.
- **Done when:** The closest point never leaves the finite segment.

## Day 15 — Mini steering laboratory

- **Build:** A triangle seeks the mouse at constant speed. Draw desired
  direction, current velocity, and steering difference.
- **Math:** Steering compares current and desired velocity vectors.
- **Perturb:** Limit steering magnitude and observe gradual turning.
- **Done when:** You can distinguish direction, velocity, and acceleration.

### Week 3 recall

Explain projection and rejection using the words “parallel component” and
“perpendicular component.” Recreate the three canonical dot-product cases.

---

# Week 4 — Linear and affine transformations

## Day 16 — Animate a translated arrow over a duration

- **Build:** Draw an arrow and translate it from A to B in exactly two seconds.
  Draw both endpoints and show normalized progress `t`.
- **Math:** Translation is affine; the displacement is `(B - A) * t`.
- **Perturb:** Change duration without changing endpoints.
- **Done when:** The arrow arrives at B at the requested time and retains shape.

## Day 17 — Scale around a chosen pivot

- **Build:** Scale an arrow from 0.5 to 2.0 around its tail, then around its
  center. Draw the pivot.
- **Math:** Move to pivot-local coordinates, scale, then move back.
- **Perturb:** Use negative scale and explain the reflection.
- **Done when:** The selected pivot remains stationary.

## Day 18 — Rotate over a fixed duration

- **Build:** Rotate an arrow exactly 270 degrees over three seconds. Preserve
  its pivot and length.
- **Math:** Interpolate an angle, then apply the rotation transform.
- **Perturb:** Compare clockwise and counterclockwise sign conventions.
- **Done when:** Final orientation matches your pre-drawn target arrow.

## Day 19 — Transformation order

- **Build:** Draw the same triangle twice: scale-then-translate and
  translate-then-scale. Use distinct colors.
- **Math:** Transform composition is generally not commutative.
- **Perturb:** Add rotation and try two more orders.
- **Done when:** You can predict why translating first also scales translation.

## Day 20 — Local space and world space

- **Build:** Create a local arrow shape around `(0,0)`, then place several
  transformed instances in world/screen space.
- **Math:** Geometry can be stored once locally and transformed many times.
- **Perturb:** Give each instance a different basis, scale, and translation.
- **Done when:** No instance requires separately authored vertices.

### Week 4 recall

Explain why rotation about the origin is linear but translation is affine. Give
one example where changing transform order changes the picture.

---

# Week 5 — Interpolation and nonlinear animation

## Day 21 — Linear interpolation

- **Build:** Animate three points between A and B using direct elapsed time,
  clamped normalized time, and repeated frame-to-frame lerp.
- **Math:** Compare finite-duration interpolation with exponential convergence.
- **Perturb:** Run at different FPS and compare arrival behavior.
- **Done when:** You can explain why repeated lerp may never exactly arrive.

## Day 22 — Smoothstep

- **Build:** Move two arrows A-to-B: one linear and one using smoothstep. Plot
  each progress curve in a small graph.
- **Math:** Smoothstep remaps time nonlinearly while endpoints remain fixed.
- **Perturb:** Inspect velocity near `t=0` and `t=1`.
- **Done when:** You can see easing without changing total duration.

## Day 23 — Compare easing families

- **Build:** Animate four identical shapes using linear, ease-in, ease-out, and
  ease-in-out timing. Label each lane.
- **Math:** Easing transforms the time parameter, not necessarily the geometry.
- **Perturb:** Use Raylib's `reasings.h` only after implementing one polynomial.
- **Done when:** All shapes start/end together but travel differently.

## Day 24 — Sinusoidal motion

- **Build:** Make an arrow bob vertically while moving horizontally at constant
  speed. Plot its path.
- **Math:** A nonlinear position function can combine linear drift and periodic
  displacement.
- **Perturb:** Independently change amplitude, frequency, and phase.
- **Done when:** You can name which parameter changes height versus cycle rate.

## Day 25 — Parametric curves

- **Build:** Toggle a moving point among a circle, ellipse, and expanding spiral.
  Leave a short trail of sampled points.
- **Math:** X and Y are independent functions of a shared parameter.
- **Perturb:** Swap sine/cosine and reverse parameter direction.
- **Done when:** You can predict starting point and traversal direction.

### Week 5 recall

Define interpolation, extrapolation, easing, amplitude, frequency, and phase.
Explain why a nonlinear time mapping can still move along a straight line.

---

# Week 6 — Entering 3D space

## Day 26 — Draw and read a 3D basis

- **Build:** Create a fixed perspective camera, grid, origin, and X/Y/Z arrows.
  Add labels using a 2D overlay.
- **Math:** A 3D point uses three basis directions; projection maps it to 2D.
- **Perturb:** Move one axis endpoint at a time and predict screen motion.
- **Done when:** You can identify every world axis from the image.

## Day 27 — A 3D vector arrow

- **Build:** Draw an arrow from A to B using a line shaft and simple arrowhead.
  Display displacement and magnitude.
- **Math:** `B - A` now has X, Y, and Z components.
- **Perturb:** Zero two components at a time.
- **Done when:** Arrow length matches `Vector3Distance(A,B)`.

## Day 28 — 3D dot product and facing

- **Build:** Draw a surface normal and a camera direction. Change a face color
  based on their dot product.
- **Math:** Dot identifies alignment in any dimension.
- **Perturb:** Move the camera around the surface.
- **Done when:** Front/back/perpendicular cases are visually identifiable.

## Day 29 — Cross product and handedness

- **Build:** Draw vectors A, B, and `cross(A,B)` from one origin. Swap A/B with
  a key.
- **Math:** Cross produces a vector perpendicular to both inputs; order changes
  its sign.
- **Perturb:** Make A and B parallel.
- **Done when:** You can use the right-hand rule to predict the result.

## Day 30 — Build an orthonormal basis

- **Build:** From a forward direction and world-up hint, derive and draw right
  and corrected-up arrows.
- **Math:** Cross products plus normalization construct a perpendicular unit
  basis.
- **Perturb:** Make forward approach world up and observe the degeneracy.
- **Done when:** Pairwise dots are near zero and lengths are near one.

### Week 6 recall

Explain why two vectors are insufficient when they become parallel. State the
difference between world up and a camera's corrected view-up direction.

---

# Week 7 — Vertices, triangles, planes, and bounds

## Day 31 — Triangle vertices and winding

- **Build:** Draw one 3D triangle from three vertices. Add a key that swaps two
  vertices and label clockwise/counterclockwise winding.
- **Math:** Vertex order determines orientation and a consistent face normal.
- **Perturb:** View both sides with a movable camera.
- **Done when:** You can predict how swapping two vertices flips orientation.

## Day 32 — Compute and draw a triangle normal

- **Build:** Compute two triangle edges, cross them, normalize the result, and
  draw the normal from the triangle centroid.
- **Math:** A face normal comes from the cross product of two nonparallel edges.
- **Perturb:** Change edge order and make the triangle nearly degenerate.
- **Done when:** Normal direction agrees with winding.

## Day 33 — Plane equation and signed distance

- **Build:** Draw a plane patch, its normal, a movable point, and a line showing
  signed distance to the plane.
- **Math:** Dotting point displacement with a unit normal yields signed distance.
- **Perturb:** Move the point to both sides and onto the plane.
- **Done when:** Sign changes exactly when the point crosses the plane.

## Day 34 — Project a point onto a plane

- **Build:** Add the closest point on the plane and visualize the removed normal
  component.
- **Math:** Subtract signed-distance times the unit normal.
- **Perturb:** Tilt the plane and move the source point.
- **Done when:** The projected point's signed distance is approximately zero.

## Day 35 — Construct cube vertices and edges

- **Build:** Given center and half-extents, calculate eight vertices and draw
  twelve edges with an explicit index table.
- **Math:** Every vertex is center plus a signed combination of half-extents.
- **Perturb:** Use nonuniform dimensions and translate the center.
- **Done when:** Your wire cube matches `DrawCubeWiresV`.

### Week 7 recall

From memory, explain winding, edge vector, normal, centroid, plane, signed
distance, center, dimensions, and half-extents.

---

# Week 8 — Camera geometry: yaw, pitch, orbit, and roll

## Day 36 — Derive a look-at basis

- **Build:** Given camera position, target, and world-up hint, draw forward,
  right, and corrected up at the camera.
- **Math:** Camera orientation is a basis derived from directions, not a physical
  object in the scene.
- **Perturb:** Move target and camera independently.
- **Done when:** The three basis vectors remain perpendicular and normalized.

## Day 37 — Yaw-only orbit

- **Build:** Orbit a camera around a target at constant height and distance.
  Draw the horizontal radius and target.
- **Math:** Yaw distributes one radius across X and Z using sine/cosine.
- **Perturb:** Verify yaw 0, pi/2, pi, and 3pi/2.
- **Done when:** Radius remains constant for a full revolution.

## Day 38 — Pitch-only orbit slice

- **Build:** Hold yaw fixed and animate pitch between safe limits. Draw
  horizontal radius, vertical offset, and total distance.
- **Math:** Pitch splits distance into cosine-adjacent and sine-opposite parts.
- **Perturb:** Test zero and positive/negative angles; avoid exactly +/-pi/2.
- **Done when:** Total camera-target distance remains constant.

## Day 39 — Combined yaw and pitch orbit

- **Build:** Store target, yaw, pitch, and distance; derive `Camera3D` every
  frame. Drive yaw with time and pitch with keys.
- **Math:** Pitch creates horizontal radius; yaw rotates it in XZ.
- **Perturb:** Use a nonzero target to prove all offsets are target-relative.
- **Done when:** Camera follows a sphere around the target without drift.

## Day 40 — Camera roll

- **Build:** Keep position and target fixed while rotating the camera up vector
  around its forward direction.
- **Math:** Roll changes view-basis orientation without changing where the
  camera sits or looks.
- **Perturb:** Compare 0, 45, 90, and 180 degrees.
- **Done when:** The horizon rotates while the target remains centered.

### Week 8 recall

State precisely which orbit variables determine position and which Raylib value
determines what the camera looks at. Explain yaw, pitch, and roll without using
mouse-motion language.

---

# Week 9 — Camera movement and projection

## Day 41 — Dolly, truck, and pedestal

- **Build:** Add keys for camera-forward movement, camera-right movement, and
  world-up movement. Draw the movement basis.
- **Math:** Movement direction can be expressed in camera basis or world basis.
- **Perturb:** Decide whether target moves with camera for each mode.
- **Done when:** You can distinguish orbit, dolly, truck, and pedestal motion.

## Day 42 — Dolly versus field of view

- **Build:** Show two viewports or toggle between moving the camera closer and
  narrowing FOV until one object has similar apparent size.
- **Math:** Dolly changes spatial relationships; FOV changes projection.
- **Perturb:** Add a foreground and background object to expose the difference.
- **Done when:** You can explain the “dolly zoom” relationship.

## Day 43 — Perspective versus orthographic

- **Build:** Draw equal cubes at several depths and toggle projection type.
  Keep camera orientation fixed.
- **Math:** Perspective scales apparent size with depth; orthographic does not.
- **Perturb:** Use a non-top-down camera to break the common misconception.
- **Done when:** You can predict cube sizes in both modes.

## Day 44 — Camera-relative navigation

- **Build:** Move a marker forward/right relative to camera view while keeping
  it on the XZ plane.
- **Math:** Project camera forward onto XZ, normalize, and derive right.
- **Perturb:** Raise/lower camera pitch and observe why vertical removal matters.
- **Done when:** Controls remain visually forward/right after yaw changes.

## Day 45 — Visualize a screen-to-world ray

- **Build:** Use Raylib to create a mouse ray and draw its origin/direction into
  the world. Add a distant endpoint for visualization.
- **Math:** A screen pixel becomes a ray because depth is not known.
- **Perturb:** Move cursor across corners and center in both projections.
- **Done when:** The ray visibly passes through the expected screen location.

### Week 9 recall

Explain the difference among changing camera position, changing camera target,
changing FOV, and changing projection type.

---

# Week 10 — Rays, collisions, and picking

## Day 46 — Ray-plane intersection

- **Build:** Intersect the mouse ray with the ground plane and draw a marker at
  the hit point.
- **Math:** Solve for ray parameter `t` where the ray reaches the plane.
- **Perturb:** Aim parallel to and behind the plane; reject invalid hits.
- **Done when:** The marker follows the cursor across the ground.

## Day 47 — Ray-sphere intersection

- **Build:** Place several spheres and highlight the nearest ray hit. Draw the
  hit point and normal.
- **Math:** Substitute the ray equation into the sphere equation.
- **Perturb:** Test misses, tangents, inside starts, and two intersections.
- **Done when:** Nearest positive distance wins.

## Day 48 — Ray-AABB intersection

- **Build:** Pick several axis-aligned boxes using Raylib, then implement or
  diagram the slab reasoning for one box.
- **Math:** Each axis restricts the valid interval of ray parameter values.
- **Perturb:** Test rays parallel to box faces and origins inside boxes.
- **Done when:** You understand why all three intervals must overlap.

## Day 49 — Closest point on an AABB

- **Build:** Move a point around a box and draw the closest point on/in the box.
  Color inside and outside cases differently.
- **Math:** Clamp each coordinate independently to box bounds.
- **Perturb:** Visit faces, edges, corners, and interior.
- **Done when:** Squared distance is zero exactly inside the box.

## Day 50 — A complete picking loop

- **Build:** Draw ten boxes, test all against the mouse ray, and select the
  nearest hit. Display index and hit distance.
- **Math:** Screen overlap does not determine spatial proximity; collision
  distance does.
- **Perturb:** Arrange boxes so several overlap on screen.
- **Done when:** Selection always chooses the visibly frontmost hit.

### Week 10 recall

Describe a ray as origin plus direction times parameter. Explain why picking
must reject negative parameters and compare all positive intersections.

---

# Week 11 — Matrices, composition, hierarchy, and inverse transforms

## Day 51 — Transform one vertex with a matrix

- **Build:** Display a local point and its transformed world point under a
  translation/rotation/scale matrix. Draw the connecting construction.
- **Math:** Homogeneous coordinates allow affine transforms in matrix form.
- **Perturb:** Apply each transform alone before composing them.
- **Done when:** Manual expectations match `Vector3Transform`.

## Day 52 — Matrix multiplication order

- **Build:** Render the same local axes with `T*R*S` and another order. Label
  each result.
- **Math:** Matrix composition order encodes which coordinate space each
  operation acts in.
- **Perturb:** Use a large translation and nonuniform scale.
- **Done when:** You can predict which order moves the pivot.

## Day 53 — Parent-child hierarchy

- **Build:** Make a simple sun/planet/moon hierarchy using composed transforms.
  Draw every local origin and basis.
- **Math:** Child world transform combines parent world and child local
  transforms.
- **Perturb:** Rotate parent and child independently.
- **Done when:** The moon follows the planet while retaining its own orbit.

## Day 54 — Inverse transforms

- **Build:** Transform a world-space point into an object's local space, perform
  a simple local bounds test, and draw the result.
- **Math:** An inverse transform reverses coordinate-space conversion.
- **Perturb:** Translate, rotate, and scale the object.
- **Done when:** Local-space test remains stable as the object moves.

## Day 55 — Normals under nonuniform scale

- **Build:** Transform a surface and its normal under uniform and nonuniform
  scale. Compare naïve direction transformation with inverse-transpose behavior.
- **Math:** Normals must remain perpendicular to transformed tangent directions.
- **Perturb:** Use an obvious scale such as `(3,1,0.5)`.
- **Done when:** Corrected normal remains perpendicular to the surface.

### Week 11 recall

Explain local versus world matrices, composition order, parent-child transforms,
and why inverse transforms simplify some collision tests.

---

# Week 12 — Advanced motion and capstones

## Day 56 — Euler interpolation versus quaternion slerp

- **Build:** Rotate two objects between the same orientations: one by separately
  interpolating Euler angles, one with Raymath quaternion slerp.
- **Math:** Quaternions encode orientation without choosing three sequential
  axis rotations; slerp follows a spherical path.
- **Perturb:** Choose endpoints that cross an angle wrap.
- **Done when:** You can describe the visible difference without deriving all
  quaternion algebra.

## Day 57 — Bézier camera rail

- **Build:** Move a camera target or marker along a cubic Bézier curve. Draw
  control points, control polygon, curve, and tangent arrow.
- **Math:** Weighted polynomial blending creates a smooth parametric path.
- **Perturb:** Move one control point and predict the affected region.
- **Done when:** Orientation can follow the curve tangent without jitter.

## Day 58 — Nonlinear wave deformation

- **Build:** Draw a grid of points or line segments and displace Y by a sine
  function of X, Z, and time.
- **Math:** A nonlinear spatial transform can depend on position and time.
- **Perturb:** Separate amplitude, spatial frequency, temporal frequency, and
  phase.
- **Done when:** You can freeze time and still explain the spatial shape.

## Day 59 — Depth ordering experiment

- **Build:** Draw several translucent shapes, deliberately reverse their order,
  then sort them far-to-near by camera distance.
- **Math:** Alpha blending is order-dependent even when depth testing exists.
- **Perturb:** Rotate camera and update sort keys every frame.
- **Done when:** You can explain why opaque and transparent ordering differ.

## Day 60 — Capstone: interactive 3D math laboratory

- **Build:** Combine an orbit camera, axes, one transformed object, visible
  local/world bases, a mouse ray, nearest-hit selection, and one nonlinear
  animation. Add toggles for diagnostic constructions.
- **Math:** Coordinate spaces, vectors, transforms, camera basis, projection,
  and intersections work as one pipeline.
- **Perturb:** Toggle perspective/orthographic and pause animation.
- **Done when:** You can point to every visible arrow/line and explain the
  calculation that placed it.

### Week 12 recall

Rebuild a small orbit-and-picking scene without opening prior exercises. List
three concepts that now feel intuitive and three that deserve another cycle.

---

# Formula card

Use this only after making a prediction.

## Vector basics

```text
displacement = B - A
length(v) = sqrt(dot(v, v))
unit(v) = v / length(v), when length is nonzero
distance(A, B) = length(B - A)
position_next = position + velocity * dt
```

## Dot, projection, and reflection

```text
dot(a,b) = ax*bx + ay*by (+ az*bz)
projection of v onto unit n = n * dot(v,n)
rejection = v - projection
reflection across unit normal n = v - 2*n*dot(v,n)
```

For normalized vectors, dot is the cosine of their angle.

## Cross product

```text
cross(a,b) = (
    ay*bz - az*by,
    az*bx - ax*bz,
    ax*by - ay*bx
)
```

`cross(a,b) = -cross(b,a)`. Parallel inputs produce the zero vector.

## 2D rotation

```text
x' = x*cos(angle) - y*sin(angle)
y' = x*sin(angle) + y*cos(angle)
```

Remember that screen +Y points down, which affects visual rotation direction.

## Interpolation and time

```text
t = clamp(elapsed / duration, 0, 1)
lerp(a,b,t) = a + (b - a)*t
smoothstep(t) = t*t*(3 - 2*t)
```

## Orbit camera with +Y up

With yaw zero defined on the target's +Z side:

```text
horizontal_radius = distance*cos(pitch)
vertical_offset = distance*sin(pitch)
x_offset = horizontal_radius*sin(yaw)
z_offset = horizontal_radius*cos(yaw)
camera_position = target + (x_offset, vertical_offset, z_offset)
camera_target = target
```

Useful reverse conversion:

```text
offset = camera_position - target
horizontal_radius = sqrt(offset.x^2 + offset.z^2)
distance = length(offset)
pitch = atan2(offset.y, horizontal_radius)
yaw = atan2(offset.x, offset.z)
```

## Camera basis

Exact signs depend on the forward convention; remain consistent:

```text
forward = normalize(target - position)
right = normalize(cross(forward, world_up))
view_up = cross(right, forward)
```

Forward must not be parallel to the up hint.

## Plane and ray

```text
ray(t) = origin + direction*t
signed_plane_distance = dot(point - point_on_plane, unit_normal)
projected_point = point - unit_normal*signed_plane_distance
```

For visible forward ray hits, `t` must normally be nonnegative.

## Bounds

```text
half_extents = dimensions * 0.5
minimum = center - half_extents
maximum = center + half_extents
closest_point_on_AABB = clamp(point, minimum, maximum) per axis
```

---

# Raylib API practice map

These are prompts, not requirements to use every function.

## 2D drawing and input

- `DrawLineV`, `DrawLineEx`, `DrawCircleV`, `DrawTriangle`
- `DrawText`, `TextFormat`
- `GetMousePosition`, `GetFrameTime`, `IsKeyDown`
- `GetScreenToWorld2D`, `GetWorldToScreen2D`

## 3D drawing and cameras

- `BeginMode3D`, `EndMode3D`, `DrawGrid`
- `DrawLine3D`, `DrawTriangle3D`, `DrawSphere`, `DrawCubeV`
- `GetWorldToScreen`, `GetMouseRay`

## Raymath comparison functions

- `Vector2Add`, `Vector2Subtract`, `Vector2Length`, `Vector2Normalize`
- `Vector3Add`, `Vector3Subtract`, `Vector3Length`, `Vector3Normalize`
- `Vector2DotProduct`, `Vector3DotProduct`, `Vector3CrossProduct`
- `Vector2Rotate`, `Vector3Transform`
- `MatrixMultiply`, `MatrixInvert`, `MatrixTranslate`, `MatrixRotateXYZ`
- `QuaternionFromEuler`, `QuaternionSlerp`

Implement the day's core operation manually before substituting a Raymath
helper, unless the exercise explicitly concerns Raylib's camera or collision
API.

---

# Local references

Read only the smallest relevant example after attempting the exercise:

- `vendor/raylib/src/raylib.h` — API declarations and brief descriptions
- `vendor/raylib/src/raymath.h` — vector, matrix, and quaternion formulas
- `vendor/raylib/examples/core/core_delta_time.c`
- `vendor/raylib/examples/core/core_2d_camera.c`
- `vendor/raylib/examples/core/core_world_screen.c`
- `vendor/raylib/examples/core/core_3d_camera_mode.c`
- `vendor/raylib/examples/core/core_3d_camera_free.c`
- `vendor/raylib/examples/core/core_3d_camera_first_person.c`
- `vendor/raylib/examples/core/core_3d_picking.c`
- `vendor/raylib/examples/models/models_geometric_shapes.c`
- `vendor/raylib/examples/models/models_rotating_cube.c`
- `vendor/raylib/examples/models/models_mesh_picking.c`
- `vendor/raylib/examples/models/models_waving_cubes.c`
- `vendor/raylib/examples/shapes/shapes_lines_drawing.c`
- `vendor/raylib/examples/shapes/shapes_lines_bezier.c`
- `vendor/raylib/examples/shapes/shapes_splines_drawing.c`
- `vendor/raylib/examples/shapes/shapes_easings_testbed.c`
- `vendor/raylib/examples/shapes/reasings.h`
- `REPORT.md`, especially Graphics Rendering 101 and camera sections

---

# Progress ledger

Mark a day complete only after the program runs, has been reviewed, and you
write the short reflection.

| Week | Days | Status | Notes |
|---:|---|---|---|
| 1 | 1-5 | not started | |
| 2 | 6-10 | not started | |
| 3 | 11-15 | not started | |
| 4 | 16-20 | not started | |
| 5 | 21-25 | not started | |
| 6 | 26-30 | not started | |
| 7 | 31-35 | not started | |
| 8 | 36-40 | not started | |
| 9 | 41-45 | not started | |
| 10 | 46-50 | not started | |
| 11 | 51-55 | not started | |
| 12 | 56-60 | not started | |

## Starting prompt

When beginning this companion track in a separate session, use:

> I am starting Day 1 of `3D_SPACE_CURRICULUM.md`. Coach only this 30-minute
> exercise. Give me the visible target, constraints, hints, and references so I
> can start coding immediately. Answer questions just in time, then review my
> work and ask reflection questions after it runs.


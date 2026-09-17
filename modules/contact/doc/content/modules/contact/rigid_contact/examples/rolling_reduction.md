# Two-Pass Plate Rolling Reduction

## Description

Rolling-mill demonstration for the rigid-body contact system.  A finite
plate is fed forward through two rigid rollers that reduce its thickness
in two passes: the first roller takes the plate from height 1 to height
0.75, the second from 0.75 to 0.5.  Both rollers are represented as
[InfiniteCylinderContactor.md] instances --- axes along the plate's
width direction, so the same level-set applies at every axial cross
section.

## Geometry (quarter symmetry)

Full plate before symmetry reduction: $1 \times 2 \times 4$ (height by
width by length).  The problem has two natural symmetry planes:

- $x = 0$ mid-width plane
- $y = 0$ mid-height plane

so the modeled quarter occupies $x \in [0, 1]$, $y \in [0, 0.5]$,
$z \in [-4, 0]$.  Because the rollers' axes are along $\hat{x}$ they
cross the $x = 0$ symmetry plane naturally; only the $x \ge 0$ half of
each cylinder actually enters the level-set on the modeled quarter, so
the rolling force reported by the quarter is the full-plate force
scaled by the two symmetry planes.

Roller layout (positions given in modeled coordinates; the "bottom" of
each roller is where it contacts the plate top at $y = 0.5$ after
reduction):

| Roller | axis | radius | center                | bottom $y$ | full-plate reduction |
|--------|------|--------|-----------------------|------------|----------------------|
| Roll 1 | $\hat{x}$ | 2 | $(0, 2.375, +1)$ | $0.375$ | $1.0 \to 0.75$ |
| Roll 2 | $\hat{x}$ | 2 | $(0, 2.25,\, +3)$ | $0.25$  | $0.75 \to 0.5$ |

The mesh is a straight
[GeneratedMeshGenerator](/meshgenerators/GeneratedMeshGenerator.md) hex
grid over the quarter volume.  Top-of-file variables `nx`, `ny`, `nz`
drive the resolution; the shipped defaults keep the run under two
minutes on 8 ranks and can be bumped for a converged rolling force.

## Contact setup

Both rollers hang their LM variables off the *same* shared lower-d
block on the plate's top sideset --- two independent
`[RigidContact]` sub-blocks that each emit their own
[LowerDBlockFromSidesetGenerator](/meshgenerators/LowerDBlockFromSidesetGenerator.md)
on the same sideset would collide (only the first attach would take
effect), so the lower-d block is generated up front in `[Mesh]` and
each sub-block references it with `add_lower_d_block = false`:

!listing modules/contact/examples/rigid/rolling-reduction/rolling_reduction.i block=Mesh/contact_lower

!listing modules/contact/examples/rigid/rolling-reduction/rolling_reduction.i block=RigidContact

## Material

Rate-independent J2 plasticity with mild power-law hardening; same
finite-strain Lagrangian pipeline as the ld-inelastic examples.  Yield
stress is deliberately low relative to Young's modulus
($\sigma_Y \approx E / 40$) so the plate top enters the plastic branch
as soon as it touches a roller instead of springing back elastically:

!listing modules/contact/examples/rigid/rolling-reduction/rolling_reduction.i block=UserObjects

Note that `volumetric_locking_correction = false` is required for this
input: with the low yield stress and the very large plastic volumetric
flow under the rollers, VLC's stabilized strain and the eigen-
decomposition kinematics interact to produce a nonsymmetric
intermediate rate tensor and abort with "The tensor is not symmetric"
in `FactorizedRankTwoTensor`.  A finer mesh in the through-thickness
direction than the shipped default would let a user re-enable VLC.

## Boundary conditions and load

Two symmetry-plane
[DirichletBCs](/BCs/DirichletBC.md), plus a
[FunctionDirichletBC](/BCs/FunctionDirichletBC.md) on the back face that
ramps `disp_z` from $0$ to `push_distance = 4` over the run --- long
enough to feed the plate past both rollers with margin:

!listing modules/contact/examples/rigid/rolling-reduction/rolling_reduction.i block=BCs

## Solver

Plain Newton + LU + `basic` line search, no bounds; the plate top only
ever pushes *against* the rollers (the rollers can't pull), so $\lambda
\ge 0$ is enforced physically and the SSLS+bounds machinery is not
required.  `IterationAdaptiveDT` reduces the step size across the
transient contact events (plate-front first touch on each roller).

## Full input

!listing modules/contact/examples/rigid/rolling-reduction/rolling_reduction.i

## Results

!media media/contact/rigid_contact/rolling_reduction.gif
       id=fig:rolling_reduction
       caption=Rolling reduction animation, viewed along the roller axis
               ($+x$ into the page).  The plate enters the frame from the
               left at rest; as the back face is driven forward the plate
               top encounters roller 1 (thickness reduced to 0.75), then
               continues into roller 2 (thickness reduced to 0.5).
               `plastic_strain_mag` on the deformed shape rises from 0
               (dark blue) to $\sim 1.3$ (red) in two bands corresponding
               to each roller's pass.

## Suggested visualization

- Cutaway of the deformed plate with `plastic_strain_mag` on a
  diametral slice, showing the accumulated shear bands under each
  roller pass.
- `max_lm_roll1` and `max_lm_roll2` versus time --- the two peak
  contact-pressure histories rise as the plate front encounters each
  roller and settle to a quasi-steady value once steady rolling is
  established.
- Contour of `normal_lm_roll1` / `normal_lm_roll2` on the top face to
  visualize the elongated contact footprints along the axial direction.

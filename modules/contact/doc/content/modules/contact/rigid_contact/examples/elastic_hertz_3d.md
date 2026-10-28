# Elastic Hertz Contact, 3D Quarter Symmetry

## Description

Rigid sphere pressed into an elastic quarter-sphere.  This is the
introductory rigid-body contact example --- displacement-controlled,
small-strain, linear elasticity, and an analytic sphere contactor ---
and is also the workhorse regression case for the formulation.

Symmetry planes are placed at $x = 0$ and $z = 0$, so the model is one
quarter of a sphere-on-sphere Hertzian setup:

- Subdomain `1` is the deformable quarter-sphere, $E = 1.40625\times 10^7$,
  $\nu = 0.25$, radius 2.  Its curved bottom carries sideset `100`; the top
  face carries sideset `2` and is driven by a `FunctionDirichletBC`.
- The analytic [SphereContactor.md] plays the role of the rigid indenter.
- The sidesets `1` and `3` are the symmetry planes.

The mesh is built entirely from
[SphereMeshGenerator](/meshgenerators/SphereMeshGenerator.md) plus three
[PlaneDeletionGenerator](/meshgenerators/PlaneDeletionGenerator.md)s to
carve out the quarter-hemisphere, with sideset ids assigned by
[SideSetsFromNormalsGenerator](/meshgenerators/SideSetsFromNormalsGenerator.md).
The top-of-file variable `sphere_refinement` (default `3`) is passed through
to `SphereMeshGenerator`'s `nr`: increment it to refine the mesh globally.
`nr = 3` gives ~450 hex elements per octant and runs in a few seconds; the
Hertzian peak pressure is concentrated in a small pole cap, so users
comparing quantitative Hertz values should bump `nr` up (`nr = 4` gives
~3600 elements per octant).

The Hertz reference (with $E^\ast = 1.5\times 10^7$, $R_{\text{eff}} = 1$)
predicts, at $\text{depth}\ d = 0.01$, contact radius
$a = \sqrt{R d} = 0.1$ and peak pressure
$p_0 = 2 E^\ast a / (\pi R) = 9.55\times 10^5$.  The regression output
reproduces both to within mesh discretization error.

## Contact setup

The [`[RigidContact]`](syntax/RigidContact/index.md) sub-block expands to
the full analytic-level-set stack:
[LowerDBlockFromSidesetGenerator](/meshgenerators/LowerDBlockFromSidesetGenerator.md),
[RigidBodyContactSparsity.md], the `normal_lm` variable with
`ConstantBounds`, [RigidBodyNodalNCPKernel.md],
[RigidBodyNormalMechanicalContact.md] on each displacement component, the
problem-coverage relaxation, and an SMP full preconditioner.  The
executioner uses PETSc `SNESVINEWTONSSLS` to enforce $\lambda \ge 0$.

!listing modules/contact/examples/rigid/elastic/hertz_elastic_3d.i block=RigidContact

## Full input

!listing modules/contact/examples/rigid/elastic/hertz_elastic_3d.i

## Results

!media media/contact/rigid_contact/elastic_hertz_3d.png
       id=fig:elastic_hertz
       caption=Elastic Hertz results at end_time = 1.  Left: `stress_zz`
               on the deformed quarter-hemisphere, with the entire model
               in gentle compression (dark red) and stress concentrated
               against the rigid indenter at the bottom pole (blue).
               Right: numerical `contactor_force` (red) plotted against
               the analytic Hertz reference `hertz_force_analytic`
               (green) as a function of `contactor_displacement`, showing
               close agreement over the full load history.  Run at
               `sphere_refinement = 5` (finer than the shipped default of
               3) to converge the Hertz peak.

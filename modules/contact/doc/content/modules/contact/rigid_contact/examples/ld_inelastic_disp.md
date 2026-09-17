# Large-Deformation J2 Plasticity, Displacement Control

## Description

Same geometry, mesh, and refinement knob (`sphere_refinement`) as the
[elastic Hertz 3D example](modules/contact/rigid_contact/examples/elastic_hertz_3d.md)
--- rigid sphere pressed into a quarter-sphere of deformable material at
$x, z$ symmetry --- but with the deformable body upgraded to a
finite-strain J2 plasticity model with linear power-law hardening.  The
top face is pushed down by a `FunctionDirichletBC` ramping from $0$ to
$-0.1$; the load is a factor of ten larger than the elastic case, deep
enough to develop a large plastic zone underneath the indenter.

## Contact setup

The [`[RigidContact]`](syntax/RigidContact/index.md) sub-block is
identical to the elastic case: analytic [SphereContactor.md] +
[RigidBodyNodalNCPKernel.md] + [RigidBodyNormalMechanicalContact.md] +
bounds and sparsity.  The example is entirely disp-controlled --- no
`force` parameter --- so no [RigidBodyLoadControl.md] is emitted.

!listing modules/contact/examples/rigid/ld-inelastic/hertz_inelastic_finite_3d.i block=RigidContact

## Solid mechanics setup

The interesting part of this example is the interaction between the
rigid-body contact stack and the new-Lagrangian solid mechanics
pipeline needed for finite-strain plasticity.  The input uses
`[Physics/SolidMechanics/QuasiStatic]` with `compatibility_mode = true`,
which auto-wraps the user's stress material with
`ComputeLagrangianWrappedStress` and creates a `ComputeLagrangianStrain`
using the `rashid_eigen` kinematic approximation.  In `[Materials]` the
user therefore supplies only the elasticity tensor, the
`ComputeMultiPlasticityStress` (published as `stress`), and any
auxiliary invariants.

!listing modules/contact/examples/rigid/ld-inelastic/hertz_inelastic_finite_3d.i block=Physics/SolidMechanics/QuasiStatic

The displacement variables are declared explicitly in `[Variables]`
(rather than through the Physics action) because they must span both
the parent block `1` and the lower-d contact block created by the
rigid-contact action:

!listing modules/contact/examples/rigid/ld-inelastic/hertz_inelastic_finite_3d.i block=Variables

## Full input

!listing modules/contact/examples/rigid/ld-inelastic/hertz_inelastic_finite_3d.i

## Suggested visualization

- Contour of `plastic_strain_mag` at the last time step, showing the
  characteristic hemispherical plastic zone under the indent.
- Contour of `normal_lm` on the contact face --- the pressure
  distribution is qualitatively similar to Hertz early on and flattens
  as plasticity develops.

Rendered images will be added in a follow-up commit.

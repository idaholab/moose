# Rigid-Body Contact

The rigid-body contact system is a self-contained formulation in the contact
module for problems in which one member of the contact pair is truly rigid.
It represents the rigid body implicitly with a signed-distance (level-set)
function rather than as a discretized surface, which avoids the geometric
search that node-face and mortar contact rely on and produces a coupled
system that is often easier to solve.

Two families of level-set contactors ship with the module:

- Analytic primitives ([SphereContactor.md], [InfiniteCylinderContactor.md]).
- Triangulated surfaces read from an STL file
  ([SurfaceMeshContactor.md]).

Both derive from [LevelSetContactor.md] and are driven pointwise; a new
analytic primitive can be added by supplying the signed distance and its
gradient.  When the deformable body's contact face crosses a contactor, the
nodal min-NCP constraint

!equation
\min(g(x),\ \lambda(x)) = 0

is discretized directly by a [RigidBodyNodalNCPKernel.md], and the resulting
contact tractions are transferred to the deformable body's displacement
degrees of freedom by [RigidBodyNormalMechanicalContact.md].  Because the
contactor's geometry is available in closed form on both sides of the
interface, the ("penetrated") branch of the NCP produces a well-defined
Jacobian without needing to keep an explicit active set.

## Force control

An optional force-control layer replaces the contactor's Cartesian
translation with a scalar unknown and drives it with an integrated reaction
constraint (see [RigidBodyLoadControl.md]).  The same layer supports the
Uzawa-style outer solver ([UzawaTransient.md]) and a contact-region
warm-start predictor ([RigidBodyContactPredictor.md]) that together are
required for the hardest force-controlled inputs.

## Rigid contact syntax block

The user-facing setup is almost always through a single
[`[RigidContact]`](syntax/RigidContact/index.md) sub-block that expands
into the mesh generator, variables, kernels, boundary conditions, sparsity
user object, and preconditioner needed for the chosen formulation.  A
typical input pairs one `[RigidContact]` block with a
`[Physics/SolidMechanics/QuasiStatic]` block for the deformable body and
looks like

```
[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    add_variables = true
    block = 1
  []
[]

[UserObjects]
  [sphere]
    type = SphereContactor
    center = '0 -4 0'
    radius = 2.0
  []
[]

[RigidContact]
  [top]
    contactor = sphere
    boundary  = 100
    displacements = 'disp_x disp_y disp_z'
  []
[]
```

## Contents

- [Theory](modules/contact/rigid_contact/theory.md) --- the level-set
  contact formulation, load-control extension, Uzawa outer loop,
  warm-start predictor, sparsity augmentation, and the SDF sign
  computation for STL contactors.

### Worked examples

- [Elastic Hertz, 3D quarter symmetry](modules/contact/rigid_contact/examples/elastic_hertz_3d.md)
  --- introductory displacement-controlled elastic problem against an
  analytic sphere; used as the workhorse regression case for the
  formulation.
- [Large-deformation J2 plasticity, displacement control](modules/contact/rigid_contact/examples/ld_inelastic_disp.md)
  --- the same geometry with a finite-strain J2 material, showing the
  interaction with the new-Lagrangian solid mechanics pipeline.
- [Large-deformation J2 plasticity, force control](modules/contact/rigid_contact/examples/ld_inelastic_force.md)
  --- adds the force-control layer: the indenter position becomes a scalar
  unknown driven by a prescribed reaction.
- [Vickers indentation with an STL contactor](modules/contact/rigid_contact/examples/material_into_indenter.md)
  --- combines an STL-defined Vickers pyramid, force control, and the
  Uzawa outer solver on a plastic body.
- [Two-pass plate rolling reduction](modules/contact/rigid_contact/examples/rolling_reduction.md)
  --- multi-contactor demo: a plate fed through two
  [InfiniteCylinderContactor.md] rollers that reduce its thickness in
  two passes, with quarter symmetry across the axial and mid-height
  planes.

### Syntax

- [`[RigidContact]` sub-block](syntax/RigidContact/index.md) and
  [RigidContactAction.md] --- the recommended top-level entry point.
  This action expands into every object the formulation needs; only
  the deformable-body physics and the contactor user object stay in
  the input.

Individual objects are also documented and can be composed by hand for
advanced setups:

- [LevelSetContactor.md] / [SphereContactor.md] / [InfiniteCylinderContactor.md] / [SurfaceMeshContactor.md]
- [RigidBodyNodalNCPKernel.md]
- [RigidBodyNormalMechanicalContact.md]
- [RigidBodyLoadControl.md]
- [RigidBodyContactPredictor.md]
- [RigidBodyContactSparsity.md]
- [UzawaTransient.md]
- [LevelSetContactorAux.md]

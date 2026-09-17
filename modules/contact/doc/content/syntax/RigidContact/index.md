# RigidContact

## Description

The `[RigidContact]` block is the recommended entry point for the
[rigid-body contact system](modules/contact/rigid_contact/index.md).
Each `[<name>]` sub-block is consumed by a [RigidContactAction.md] which
expands into every object the analytic-level-set contact stack needs:

- `LowerDBlockFromSidesetGenerator` on the contact sideset (skip with
  [!param](/RigidContact/RigidContactAction/add_lower_d_block) = false
  if the mesh already carries a lower-d block).
- [RigidBodyContactSparsity.md] user object (skip with
  [!param](/RigidContact/RigidContactAction/add_sparsity_uo) = false).
- The `normal_lm` variable on the new lower-d block and (unless
  [!param](/RigidContact/RigidContactAction/enforce_bounds) is set to
  `false`) an aux `bounds_dummy` variable + `ConstantBounds` aux
  kernel that keep $\lambda \ge 0$.
- One [RigidBodyNodalNCPKernel.md] per contact sideset.
- One [RigidBodyNormalMechanicalContact.md] per displacement component.
- Problem-coverage relaxation (unless
  [!param](/RigidContact/RigidContactAction/set_problem_coverage_flags)
  is `false`) and a default SMP full preconditioner (unless
  [!param](/RigidContact/RigidContactAction/add_full_smp) is `false` or
  a `[Preconditioning]` block is already present).

Setting [!param](/RigidContact/RigidContactAction/force) on a sub-block
additionally activates the force-control layer described in the
[theory page](modules/contact/rigid_contact/theory.md#load-control):
a [NodalArea](/userobjects/NodalArea.md) UO, a scalar variable driving
the contactor's translation along
[!param](/RigidContact/RigidContactAction/load_direction), and a
[RigidBodyLoadControl.md] scalar kernel with the target-reaction
constraint.  The preconditioning shift on the load-control diagonal is
supplied either as a constant through
[!param](/RigidContact/RigidContactAction/kss_stiffness) or as a
function of time through
[!param](/RigidContact/RigidContactAction/kss_stiffness_function)
(mutually exclusive; both are ignored when
[!param](/RigidContact/RigidContactAction/force) is unset).

The *contactor* user object is not created by the action --- users
declare it themselves in `[UserObjects]` and pass its name in
[!param](/RigidContact/RigidContactAction/contactor).  See
[SphereContactor.md] and [SurfaceMeshContactor.md] for the two
concrete choices, and [LevelSetContactor.md] for the interface that
new analytic primitives should implement.

## Example input

Displacement-controlled analytic sphere:

!listing modules/contact/examples/rigid/elastic/hertz_elastic_3d.i block=RigidContact

Force-controlled analytic sphere:

!listing modules/contact/examples/rigid/ld-inelastic-force/hertz_inelastic_finite_3d_force.i block=RigidContact

Force-controlled STL contactor with UzawaTransient (see the
[Vickers example](modules/contact/rigid_contact/examples/material_into_indenter.md)):

!listing modules/contact/examples/rigid/stl-contact/material_into_indenter.i block=RigidContact

## Multiple contact interfaces

Each sub-block generates its own set of objects, keyed by the
sub-block name.  A single input can therefore host several
independent contact interfaces --- each with its own contactor,
sideset, LM variable, and (optionally) load-control scalar --- by
listing them side by side:

```
[RigidContact]
  [top]
    contactor = sphere_top
    boundary  = top
    displacements = 'disp_x disp_y disp_z'
  []
  [bottom]
    contactor = sphere_bottom
    boundary  = bottom
    displacements = 'disp_x disp_y disp_z'
    lower_d_block_id = 10002
  []
[]
```

The action derives per-sub-block default names for the LM variable,
the lower-d block, the scalar variable (force-controlled), and the
nodal-area variable, and exposes `_name`-suffixed overrides for each
if the defaults clash.  The
[!param](/RigidContact/RigidContactAction/lower_d_block_id) parameter
*must* be unique across sub-blocks; the defaults are `normal_lm_<name>`,
`contact_lower_<name>`, `indenter_<axis>`, and `nodal_area_<name>`.

## Parameters

!syntax parameters /RigidContact/RigidContactAction

!syntax inputs /RigidContact/RigidContactAction

!syntax children /RigidContact/RigidContactAction

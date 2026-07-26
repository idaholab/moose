# Solid Mechanics QuasiStatic Physics System

!syntax description /Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics

The QuasiStatic Physics is the recommended way to set up a static or quasi-static solid mechanics
problem.  From a compact set of parameters it creates the displacement variables, the strain
calculator, the [stress-divergence kernels](solid_mechanics/BalanceOfLinearMomentum.md), and the
requested outputs -- consistently and with the correct settings -- so you do not have to assemble
these objects by hand.

The parameters are organized into the groups below; each group points to the reference documentation
for the underlying theory.  The complete parameter list appears at the bottom of this page.

## Variables id=variables

Declare the displacement field the problem solves for (and any coupled temperature).  Set
[!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/add_variables) to have
the action create the displacement variables at the order matching the mesh.

- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/displacements)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/add_variables)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/temperature)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/scaling)

## Strain id=strain

Choose the kinematic model, apply eigenstrains, correct volumetric locking, and tune the strain
time integration.  `strain = FINITE` selects large-deformation kinematics and `SMALL` selects
small-deformation kinematics; eigenstrains are subtracted from the total strain;
`volumetric_locking_correction` applies the $\bar{F}$ stabilization for linear elements; and the
kinematic approximation and generalized-midpoint weight control how the incremental deformation is
integrated.  See [Kinematics](solid_mechanics/Kinematics.md),
[Kinematic Approximations](solid_mechanics/KinematicApproximations.md),
[Stabilization](solid_mechanics/Stabilization.md), and
[Generalized Midpoint Rule](solid_mechanics/GeneralizedMidpointRule.md).

- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/strain)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/volumetric_locking_correction)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/volumetric_locking_correction_mode)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/kinematic_approximation)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/generalized_midpoint_alpha)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/eigenstrain_names)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/automatic_eigenstrain_names)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/strain_base_name)

## Lagrangian Formulation id=lagrangian-formulation

Select the total or updated Lagrangian formulation.  The two produce identical results, so choose
whichever is more convenient (homogenization requires `TOTAL`).  See
[Balance of Linear Momentum](solid_mechanics/BalanceOfLinearMomentum.md).

- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/new_system)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/formulation)

## Homogenization id=homogenization

Impose cell-average stress or strain constraints on a periodic unit cell (total Lagrangian only).
See [Homogenization](solid_mechanics/Homogenization.md).

- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/constraint_types)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/targets)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/homogenized_off_diagonal_jacobian)

## Output id=output

Add AuxVariables that output stress and strain tensor components and derived scalar quantities.  See
[Visualizing Tensor Components](solid_mechanics/VisualizingTensors.md) for the available quantities.

- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/generate_output)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/material_output_order)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/material_output_family)

## Plane Stress and Plane Strain id=planar

Impose out-of-plane conditions for two-dimensional models.

- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/planar_formulation)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/scalar_out_of_plane_strain)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/out_of_plane_pressure)

## Advanced id=advanced

Subdomain restriction, automatic differentiation, and residual/Jacobian output.

- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/block)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/base_name)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/use_automatic_differentiation)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/global_strain)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/save_in)
- [!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/diag_save_in)

## Example Input File Syntax id=examples

A small deformation, total Lagrangian problem:

!listing modules/solid_mechanics/test/tests/lagrangian/cartesian/total/action/action_L.i block=Physics/SolidMechanics/QuasiStatic

A large deformation problem with homogenization constraints:

!listing modules/solid_mechanics/test/tests/lagrangian/cartesian/total/homogenization/action/action_3d.i block=Physics/SolidMechanics/QuasiStatic

A single sub-block sets up the mechanics everywhere; use multiple sub-blocks (with `block`) for
different models on different subdomains:

!listing modules/solid_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Physics/SolidMechanics/QuasiStatic

!listing modules/solid_mechanics/test/tests/action/two_block_new.i block=Physics/SolidMechanics/QuasiStatic

Parameters supplied at the `[Physics/SolidMechanics/QuasiStatic]` level act as defaults for its
sub-blocks.

!alert note
With `automatic_eigenstrain_names = true`, eigenstrain names are populated only under restrictive
conditions for classes such as [CompositeEigenstrain](CompositeEigenstrain.md),
[ComputeReducedOrderEigenstrain](ComputeReducedOrderEigenstrain.md), and
[RankTwoTensorMaterialADConverter](MaterialADConverter.md).  Set `automatic_eigenstrain_names = false`
and list the names manually if these components need to be included.

!syntax parameters /Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics

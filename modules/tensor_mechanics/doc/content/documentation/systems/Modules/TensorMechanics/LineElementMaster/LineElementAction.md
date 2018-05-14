# LineElement Action

!syntax description /Modules/TensorMechanics/LineElementMaster/LineElementAction

The LineElement Action is a convenience object that simplifies part of the
mechanics/dynamics system setup for beam and truss elements.

## Truss elements

A truss element is chosen by setting `truss = true` in the input block. The following objects can be set up for a truss element by the LineElement Action:

- Sets up displacement variables (if `add_variables` is set to true)
- Adds StressDivergence Kernels ([StressDivergenceTensorsTruss](/StressDivergenceTensorsTruss.md))

### Example Input File Syntax (Truss Elements)

!listing modules/tensor_mechanics/test/tests/truss/truss_3d_action.i block=Modules/TensorMechanics/LineElementMaster

## Beam elements

By default, the LineElement Action sets up objects required for a beam element. The following objects can be created by the action for beam elements:

- Sets up displacement and rotation variables (if `add_variables` is set to true).
- Adds StressDivergence Kernels ([StressDivergenceBeam](/StressDivergenceBeam.md))
- Adds Beam materials - [ComputeIncrementalBeamStrain](/ComputeIncrementalBeamStrain.md) or [ComputeFiniteBeamStrain](/ComputeFiniteBeamStrain.md) is set up based on the strain and rotation calculation type.
- Correctly sets `use_displaced_mesh` parameter in kernels, nodalkernels and materials
- Sets up AuxVariables and Auxkernels for translational and rotational velocities and accelerations using [NewmarkVelAux](/NewmarkVelAux.md) and [NewmarkAccelAux](/NewmarkAccelAux.md) (if `dynamic_consistent_inertia`, `_dynamic_nodal_translational_inertia` or `dynamic_nodal_rotational_inertia` is set to true)
- Sets up inertia kernels ([InertialForceBeam](/InertialForceBeam.md)) to calculate inertial forces/moments using consistent mass/inertia matrices (if `dynamic_consistent_inertia` is set to true).
- Sets up nodal translational inertia nodal kernels ([NodalTranslationalInertia](/NodalTranslationalInertia.md)) to calculate inertial forces using nodal mass (if `dynamic_nodal_translational_inertia` is set to true).
- Sets up nodal rotational inertia nodal kernels ([NodalRotationalInertia](/NodalRotationalInertia.md)) to calculate inertial moments using nodal moment of inertia matrix (if `dynamic_nodal_rotational_inertia` is set to true).

### Example Input File Syntax (Beam Elements)

!listing modules/tensor_mechanics/test/tests/beam/action/2_block_common.i block=Modules/TensorMechanics/LineElementMaster

## Subblocks

The subblocks of the LineElement action are what trigger MOOSE objects to be built.
If none of the mechanics is subdomain restricted a single subblock will be used

```
[Modules/LineElement]
  [./all]
    ...
  [../]
[]
```

if different mechanics models are needed, multiple subblocks with subdomain restrictions
can be used.

```
[Modules/LineElement]
  [./block_a]
    ...
  [../]
  [./block_b]
    ...
  [../]
[]
```

## Toplevel parameters

Parameters supplied at the `[Modules/TensorMechanics/LineElementMaster]` level act as
defaults for all the subblocks within that LineElement block.

!syntax parameters /Modules/TensorMechanics/LineElementMaster/LineElementAction

# Line Element Action System

!syntax description /Modules/TensorMechanics/LineElementMaster/LineElementAction

## Description

The LineElement Action is a convenience object that simplifies part of the
mechanics/dynamics system setup for beam or truss elements.

## Truss Elements: Constructed MooseObjects

The `LineElement` Action can be used to construct the kernels, strain materials, and displacement variables for a simulation using +Truss Elements+.

!alert note title=Set the Truss Parameter
A truss element is chosen by setting +`truss = true`+ in the input block.

!table id=lineElement_Truss_action_table caption=Correspondence Among Action Functionality and MooseObjects for the `LineElement` Action used in a +Truss Elements+ Simulation
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Create truss elements | (*none*) | `truss = true` |
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of stress divergence for a +Truss+ element | [StressDivergenceTensorsTruss](/StressDivergenceTensorsTruss.md) | `displacements` : a string of the displacement variables |

## Example Input Syntax (Truss Elements)

!listing modules/tensor_mechanics/test/tests/truss/truss_3d_action.i block=Modules/TensorMechanics/LineElementMaster

## Beam Elements: Constructed MooseObjects

By default, the `LineElement` Action sets up the kernels, strain materials, displacement and rotation variables, and auxkernels for a simulation using +Beam Elements+. It also sets the boolean `use_displaced_mesh` in a consistent manner for the kernels, nodalkernels, and materials.

!table id=lineElement_Beam_action_table caption=Correspondence Among Action Functionality and MooseObjects for the `LineElement` Action used in a +Beam Elements+ Simulation
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Add the displacement and rotation variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean
| Calculation of stress divergence for a +Beam+ element | [StressDivergenceBeam](/StressDivergenceBeam.md) | `displacements` : a string of the displacement variables |
| | | `rotations`: a string of the rotation variables |
| Calculation of strain | [ComputeIncrementalBeamStrain](/ComputeIncrementalBeamStrain.md) or [ComputeFiniteBeamStrain](/ComputeFiniteBeamStrain.md) | `strain_type`: small or large strain formulation |
| | | `rotation_type`: small or large rotation formulation |
| Calculation of inertial forces and moments with consistent mass / inertia matrices | [InertialForceBeam](/InertialForceBeam.md) | `dynamic_consistent_inertia`: boolean |
| Calculation of inertial forces using nodal mass | [NodalTranslationalInertia](/NodalTranslationalInertia.md) | `dynamic_nodal_translational_inertia`: boolean |
| Calculation of inertial moments using nodal moment of inertia matrix | [NodalRotationalInertia](/NodalRotationalInertia.md) | `dynamic_nodal_rotational_inertia`: boolean |
| Calculation of translational and rotational velocities | [NewmarkVelAux](/NewmarkVelAux.md) | `dynamic_consistent_inertia` or `dynamic_nodal_translational_inertia` or `dynamic_nodal_rotational_inertia` or `add_dynamic_variables`|
| Calculation of translational and rotational accelerations | [NewmarkAccelAux](/NewmarkAccelAux.md) | `dynamic_consistent_inertia` or `dynamic_nodal_translational_inertia` or `dynamic_nodal_rotational_inertia` or `add_dynamic_variables`|

## Example Input Syntax (Beam Elements)

!listing modules/tensor_mechanics/test/tests/beam/action/2_block_common.i block=Modules/TensorMechanics/LineElementMaster

### Subblocks

The subblocks of the LineElement action are what trigger MOOSE objects to be built.
If none of the mechanics is subdomain restricted a single subblock should be used, yet
if different mechanics models are needed, multiple subblocks with subdomain restrictions
can be used.

Parameters supplied at the `[Modules/TensorMechanics/LineElementMaster]` level act as
defaults for all the subblocks within that LineElement block.

!syntax parameters /Modules/TensorMechanics/LineElementMaster/LineElementAction

## Associated Actions

!syntax list /Modules/TensorMechanics/LineElementMaster objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/LineElementMaster objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/LineElementMaster objects=False actions=True subsystems=False

# Legacy Kernel-Only Tensor Mechanics Action

!syntax description /Kernels/TensorMechanics/LegacyTensorMechanicsAction

!alert warning title=Deprecated Action
This legacy action will soon be deprecated in favor of the more inclusive
+[TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md)+.
See the description, example use, and parameters on the
+[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md)+ action system page.

## Description

The `LegacyTensorMechanicsAction` is a convenience object that simplifies part of
the tensor mechanics system setup. It adds StressDivergence Kernels (for the
current coordinate system).

## Constructed MooseObjects

The Legacy Tensor Mechanics Action is used to construct the kernels for the
specified coordinate system.

!table id=tmMaster_action_table caption=Correspondence Among Action Functionality and MooseObjects for the Tensor Mechanics `Master` Action
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [StressDivergenceTensors](/StressDivergenceTensors.md) or [StressDivergenceRZTensors](/StressDivergenceRZTensors.md) or [StressDivergenceRSphericalTensors](/StressDivergenceRSphericalTensors.md) | `displacements` : a string of the displacement field variables |

Note that there are many variations for the calculation of the stress divergence.
Review the theoretical introduction for the
[Stress Divergence](tensor_mechanics/StressDivergence.md).
Pay particular attention to the setting of the `use_displaced_mesh` parameter
discussion; this parameter depends on the strain formulation used in the simulation.

!alert note title=Use of the Tensor Mechanics MasterAction Recommended
We recommend that users employ the +[TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md)+
whenever possible to ensure consistency between the test function gradients and
the strain formulation selected.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/elastic_patch/elastic_patch.i block=Kernels

!syntax parameters /Kernels/TensorMechanics/LegacyTensorMechanicsAction

## Associated Actions

!syntax list /Kernels/TensorMechanics objects=True actions=False subsystems=False

!syntax list /Kernels/TensorMechanics objects=False actions=False subsystems=True

!syntax list /Kernels/TensorMechanics objects=False actions=True subsystems=False

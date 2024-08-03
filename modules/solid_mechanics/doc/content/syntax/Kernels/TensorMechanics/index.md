# Legacy Kernel-Only Tensor Mechanics Action

!syntax description /Kernels/SolidMechanics/LegacyTensorMechanicsAction

!alert warning title=Deprecated Action
This legacy action is deprecated in favor of the new
+[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md)+ syntax.
See the description, example use, and parameters on the
+[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md)+ action system page.

## Description

The `LegacyTensorMechanicsAction` is a convenience object that simplifies part of
the solid mechanics system setup. It adds StressDivergence Kernels (for the
current coordinate system).

## Constructed MooseObjects

The Legacy Solid Mechanics Action is used to construct the kernels for the
specified coordinate system.

!table id=tm_quasi_static_table caption=Correspondence Among Action Functionality and MooseObjects for the Solid Mechanics QuasiStatic Physics
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [StressDivergenceTensors](/StressDivergenceTensors.md) or [StressDivergenceRZTensors](/StressDivergenceRZTensors.md) or [StressDivergenceRSphericalTensors](/StressDivergenceRSphericalTensors.md) | `displacements` : a string of the displacement field variables |

Note that there are many variations for the calculation of the stress divergence.
Review the theoretical introduction for the
[Stress Divergence](solid_mechanics/StressDivergence.md).
Pay particular attention to the setting of the `use_displaced_mesh` parameter
discussion; this parameter depends on the strain formulation used in the simulation.

!alert note title=Use of the Solid Mechanics QuasiStatic Physics Recommended
We recommend that users employ the +[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md)+
whenever possible to ensure consistency between the test function gradients and
the strain formulation selected.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/elastic_patch/elastic_patch.i block=Kernels

!syntax parameters /Kernels/SolidMechanics/LegacyTensorMechanicsAction

## Associated Actions

!syntax list /Kernels/SolidMechanics objects=True actions=False subsystems=False

!syntax list /Kernels/SolidMechanics objects=False actions=False subsystems=True

!syntax list /Kernels/SolidMechanics objects=False actions=True subsystems=False

# Compute Incremental Small Strain

!syntax description /Materials/ComputeIncrementalSmallStrain

## Description

The material `ComputeIncrementalSmallStrain` is designed for linear elasticity problems formulated
within an incremental framework.  As with [ComputeSmallStrain](/ComputeSmallStrain.md), this material
is useful for verifying material models with hand calculations because of the simplified strain
calculations.  As in the small strain material, the incremental small strain class assumes the
gradient of displacement with respect to position is much smaller than unity, and the squared
displacement gradient term is neglected in the small strain definition to give:
\begin{equation}
\epsilon = \frac{1}{2} \left( u \nabla + \nabla u \right) \quad when \quad \frac{\partial u}{ \partial x} << 1
\end{equation}
As the class name suggests, `ComputeIncrementalSmallStrain` is an incremental formulation.  The
stress increment is calculated from the current strain increment at each time step.  In this class,
the rotation tensor is defined to be the rank-2 Identity tensor: no rotations are allowed in the
model. Stateful properties, including `strain_old` and `stress_old`, are stored. This incremental
small strain material is useful as a component of verifying more complex finite incremental
strain-stress calculations.

## Example Input File Syntax

The incremental small strain calculator can be activated in the input file through the use of the
TensorMechanics Master Action, as shown below.

!listing modules/tensor_mechanics/test/tests/thermal_expansion/constant_expansion_coeff.i block=Modules/TensorMechanics

!alert note title=Use of the Tensor Mechanics Master Action Recommended
The [TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md) is designed to
automatically determine and set the strain and stress divergence parameters correctly for the
selected strain formulation.  We recommend that users employ the
[TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md) whenever possible
to ensure consistency between the test function gradients and the strain formulation selected.

Although not recommended, it is possible to directly use the `ComputeIncrementalSmallStrain` material
in the input file.

!listing modules/tensor_mechanics/test/tests/thermal_expansion/multiple_thermal_eigenstrains.i block=Materials/small_strain

!syntax parameters /Materials/ComputeIncrementalSmallStrain

!syntax inputs /Materials/ComputeIncrementalSmallStrain

!syntax children /Materials/ComputeIncrementalSmallStrain

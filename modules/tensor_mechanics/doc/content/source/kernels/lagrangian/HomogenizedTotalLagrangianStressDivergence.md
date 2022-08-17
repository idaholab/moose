# HomogenizedTotalLagrangianStressDivergence

!syntax description /Kernels/HomogenizedTotalLagrangianStressDivergence

## Overview

This object provides the total Lagrangian stress equilibrium kernel and corresponding
Jacobian for the homogenization system.  It is identical to the [`TotalLagrangianStressDivergence`](TotalLagrangianStressDivergence.md)
class except it also provides the correct off-diagonal Jacobinan
terms for the [Lagrangian kernel homogenization system](Homogenization.md).

The [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md) can add this object
automatically, which is the recommended way to set up homogenization constraints.

## Example Input File Syntax

The following example manually specifies the parameters required to setup
the kernel for a large deformation homogenization problem.
The `macro_gradient` parameter is the name of the `ScalarVariable`
containing the homogenization strain or displacement gradient field.
The `constraint_types` parameters controls the type of constraint (deformation or stress) for each input.
The [homogenization system](Homogenization.md) documentation lists the order of these inputs
for each problem dimension/type.  

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/total/homogenization/large-tests/3d.i
         block=Kernels

!syntax parameters /Kernels/HomogenizedTotalLagrangianStressDivergence

!syntax inputs /Kernels/HomogenizedTotalLagrangianStressDivergence

!syntax children /Kernels/HomogenizedTotalLagrangianStressDivergence

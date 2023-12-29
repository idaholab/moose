# HomogenizationConstraintScalarKernel

!syntax description /ScalarKernels/HomogenizationConstraintScalarKernel

## Overview

This `ScalarKernel` inputs the residual contributions from the [Lagrangian kernel homogenization system](Homogenization.md).
The [`HomogenizationConstraint`](HomogenizationConstraint.md)
`UserObject` computes the actual values of the residual volume integrals
and the [`HomogenizedTotalLagrangianStressDivergence`](HomogenizedTotalLagrangianStressDivergence.md)
Kernel handles all the off-diagonal Jacobian terms.  This
`ScalarKernel` only supplies the residual and on-diagonal Jacobian.

The [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md) can add this object
automatically, which is the recommended way to set up homogenization constraints.

## Example Input File Syntax

The following example manually specifies the parameters required to
setup the object for a 3D, large deformation homogenization problem.
In this case the name of the  [`HomogenizationConstraint`](HomogenizationConstraint.md)
is `integrator`, `ndim` sets the dimensionality of the problem,
and `large_kinematics` controls if large or small deformation kinematics
apply.

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/total/homogenization/large-tests/3d.i
         block=ScalarKernels

!syntax parameters /ScalarKernels/HomogenizationConstraintScalarKernel

!syntax inputs /ScalarKernels/HomogenizationConstraintScalarKernel

!syntax children /ScalarKernels/HomogenizationConstraintScalarKernel

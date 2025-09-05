# ADMatCoupledTimeDerivative

## Description

The `ADMatCoupledTimeDerivative` kernel is an extension of the
[`ADCoupledTimeDerivative`](/ADCoupledTimeDerivative.md) kernel that consumes a material property.

The Jacobian contribution is computed using forward mode automatic
differentiation.

## Example Syntax

The syntax is simple, taking its type
(`ADMatCoupledTimeDerivative`), the kernel variable which the
`ADMatCoupledTimeDerivative` residual is assigned to, the coupled variable `v`
that the time derivative operator acts upon, and the material property 'mat_prop'. Example syntax can be found in the
kernel block below:

!listing test/tests/kernels/mat_coupled_time_derivative/ad_mat_coupled_time_derivative_test.i block=Kernels

!syntax parameters /Kernels/ADMatCoupledTimeDerivative

!syntax inputs /Kernels/ADMatCoupledTimeDerivative

!syntax children /Kernels/ADMatCoupledTimeDerivative

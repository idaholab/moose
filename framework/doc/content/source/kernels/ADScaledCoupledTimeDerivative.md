# ADScaledCoupledTimeDerivative

## Description

The `ADScaledCoupledTimeDerivative` kernel is an extension of the
[`ADCoupledTimeDerivative`](/ADCoupledTimeDerivative.md) kernel that has a scaling factor.
This factor can be an ADReal-valued material property, set using the [!param](/Kernels/ADScaledCoupledTimeDerivative/mat_prop)
parameter. The Jacobian contribution is computed using forward mode automatic differentiation.

## Example Syntax

The syntax is simple, first with the [!param](/Kernels/ADScaledCoupledTimeDerivative/type) parameter set to
(`ADScaledCoupledTimeDerivative`), the kernel [!param](/Kernels/ADScaledCoupledTimeDerivative/variable) parameter set to the equation/variable that the
`ADScaledCoupledTimeDerivative` residual is assigned to, the coupled variable parameter [!param](/Kernels/ADScaledCoupledTimeDerivative/v) set to the variable
that the time derivative operator acts upon, and the material property [!param](/Kernels/ADScaledCoupledTimeDerivative/mat_prop).
Example syntax can be found in the kernel block below:

!listing test/tests/kernels/scaled_coupled_time_derivative/ad_scaled_coupled_time_derivative_test.i block=Kernels

!syntax parameters /Kernels/ADScaledCoupledTimeDerivative

!syntax inputs /Kernels/ADScaledCoupledTimeDerivative

!syntax children /Kernels/ADScaledCoupledTimeDerivative

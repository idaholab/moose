# MaterialDerivativeTestKernel

## Description

`MaterialDerivativeTestKernel` is a generic test `Kernel` that allows checking
the correctness of derivative material properties using the built-in Jacobian
checks in the MOOSE test harness. Its weak form is given by $$(\psi_i, p)$$
where p is a material property that depends on the governing
variables. `MaterialDerivativeTestKernel` inherits from
`DerivativeMaterialInterface` which gives it access to functions like
`getMaterialPropertyDerivative` and `mapJvarToCvar`, explained below:

- `getMaterialPropertyDerivative` takes a material property name
  (let's call it "p") and a variable name (let's call it "u") and returns a
  `MaterialProperty` equal to the derivative of the supplied material property
  with respect to the supplied variable, e.g. $\frac{\partial p}{\partial
  u}$.
- `mapJvarToCvar` maps the global variable number to the kernel's coupled variable number.

This interface allows for easy and elegant construction of correct Jacobians for
physics that depend on materials that are functions of governing variables.

## Example Syntax

The `Kernel` and `Material` blocks below demonstrate how to utilize the
`DerivativeMaterialInterface` capabilities in an input file.

- Kernel block

!listing test/tests/kernels/material_derivatives/material_derivatives_test.i block=Kernels

- Material block

!listing test/tests/kernels/material_derivatives/material_derivatives_test.i block=Materials

`MaterialDerivativeTestMaterial` takes two variables, `var1` and `var2` as input
parameters. It declares a material property with the name
`material_derivative_test_property`. It also declares two material property
derivatives that are the derivatives of `material_derivative_test_property` with
respect to `var1` and `var2`. In our test input file above, we assign `var1 = u`
and `var2 = v`. Then as long as we pass `u` and `v` as coupled variables in the
`args` parameter of `MaterialDerivativeTestKernel`, we will automatically access
the correct material property derivatives in our kernel.

This kernel puts a seleted scalar (`Real` type) material property (`material_property`) in the residual vector
and assembles the Jacobian using the derivatives of the material property as provided by the
[DerivativeMaterialInterface](/DerivativeMaterialInterface.md).

!syntax parameters /Kernels/MaterialDerivativeTestKernel

!syntax inputs /Kernels/MaterialDerivativeTestKernel

!syntax children /Kernels/MaterialDerivativeTestKernel

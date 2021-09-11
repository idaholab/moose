# ElementLpNormAux

!syntax description /AuxKernels/ElementLpNormAux

This can be used to average variables on elements, to make a copy that is constant on each
element, a monomial of order 0. The desired Lp-norm is natural conserved in this process.

This is also typically used on a variable representing an error or a difference to a known
solution, to turn an error on each quadrature point to an element-wise error.

## Alternative objects and kernels

To compute the Lp or Hp error directly from a variable and a known reference solution, use
[ElementL2ErrorFunctionAux.md] or [ElementH1ErrorFunctionAux.md] respectively.

For computing a global norm (error) metric, the [NodalL2Norm.md] and the [ElementL2Norm.md]
(respectively [NodalL2Error.md] and the [ElementL2Error.md]) postprocessors may be considered.

## Example syntax

In this example, we display several ways of creating fields computing local element-wise
norms and errors of field variables.

!listing test/tests/auxkernels/error_function_aux/error_function_aux.i block=AuxKernels

!syntax parameters /AuxKernels/ElementLpNormAux

!syntax inputs /AuxKernels/ElementLpNormAux

!syntax children /AuxKernels/ElementLpNormAux

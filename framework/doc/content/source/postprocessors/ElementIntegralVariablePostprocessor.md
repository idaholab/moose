# ElementIntegralVariablePostprocessor

The ElementIntegralVariablePostprocessor is an intermediate base class that should be derived from for any calculation involving
the integral of a variable quantity over the whole domain or a portion of the domain.

The ElementIntegralVariablePostprocessor may be used to compute the volume integral of a variable, based on the quadrature defined
for this variable. This integral may be restricted to blocks/subdomains on which the variable is defined.

## Example input file

In this input file, we use `ElementIntegralVariablePostprocessor` to compute the integral
of a variable $u$, solution of a diffusion problem, over each and all of the two subdomains it
is defined on.

!listing postprocessors/element_integral/element_block_integral_test.i

!syntax parameters /Postprocessors/ElementIntegralVariablePostprocessor

!syntax inputs /Postprocessors/ElementIntegralVariablePostprocessor

!syntax children /Postprocessors/ElementIntegralVariablePostprocessor

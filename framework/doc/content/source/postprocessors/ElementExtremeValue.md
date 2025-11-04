# ElementExtremeValue

!syntax description /Postprocessors/ElementExtremeValue

You can optionally provide a [!param](/Postprocessors/ElementExtremeValue/proxy_variable),
which will change the behavior of this postprocessor to
find the quadrature point at which the proxy variable reaches the max/min value,
and then return the value of the specified variable at that point.

This postprocessor can operate on both elemental and nodal variables. Note,
however, that since it works by sampling the variable at the quadrature points,
its returned value won't exactly match the variable's extreme value on the
domain (or blocks) the postprocessor is defined over. This is the case, unless
that extremum happens to be at a quadrature point, e.g., if the variable is
constant over any given element.
An alternative scheme is provided by [ElementExtremeFunctorValue.md], which
instead of sampling the variable at the quadrature points, samples the variable
at the centroid of each element in the domain (or blocks) of interest.
The corresponding postprocessor that finds (exact) extreme values of nodal
variables evaluated at nodes is [NodalExtremeValue.md].

## Example Input File Syntax

!listing test/tests/postprocessors/element_extreme_value/element_extreme_value.i block=Postprocessors

!syntax parameters /Postprocessors/ElementExtremeValue

!syntax inputs /Postprocessors/ElementExtremeValue

!syntax children /Postprocessors/ElementExtremeValue

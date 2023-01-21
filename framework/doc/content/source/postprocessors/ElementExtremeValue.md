# ElementExtremeValue

!syntax description /Postprocessors/ElementExtremeValue

You can optionally provide a [!param](/Postprocessors/ElementExtremeValue/proxy_variable),
which will change the behavior of this postprocessor to
find the quadrature point at which the proxy variable reaches the max/min value,
and then return the value of the specified variable at that point.

The corresponding postprocessor that finds extreme values of nodal variables
evaluated at nodes is [NodalExtremeValue](NodalExtremeValue.md)

## Example Input File Syntax

!listing test/tests/postprocessors/element_extreme_value/element_extreme_value.i block=Postprocessors

!syntax parameters /Postprocessors/ElementExtremeValue

!syntax inputs /Postprocessors/ElementExtremeValue

!syntax children /Postprocessors/ElementExtremeValue

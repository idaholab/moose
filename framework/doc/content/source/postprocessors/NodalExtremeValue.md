# NodalExtremeValue

!syntax description /Postprocessors/NodalExtremeValue

You can optionally provide a [!param](/Postprocessors/NodalExtremeValue/proxy_variable),
which will change the behavior of this postprocessor to
find the node at which the proxy variable reaches the max/min value,
and then return the value of the specified variable at that node.

The corresponding postprocessor that finds extreme values of variables evaluated
inside elements (at quadrature points) is
[ElementExtremeValue](ElementExtremeValue.md)

## Example Input File Syntax

!! Describe and include an example of how to use the NodalExtremeValue object.

!syntax parameters /Postprocessors/NodalExtremeValue

!syntax inputs /Postprocessors/NodalExtremeValue

!syntax children /Postprocessors/NodalExtremeValue

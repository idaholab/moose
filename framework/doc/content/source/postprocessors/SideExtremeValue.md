# SideExtremeValue

!syntax description /Postprocessors/SideExtremeValue

You can optionally provide a [!param](/Postprocessors/SideExtremeValue/proxy_variable),
which will change the behavior of this postprocessor to
find the point at which the proxy variable reaches the max/min value,
and then return the value of the specified variable at that node.

The corresponding postprocessor that finds extreme values of variables evaluated
inside elements (at quadrature points) is
[ElementExtremeValue](ElementExtremeValue.md)

## Example Input File Syntax

!! Describe and include an example of how to use the SideExtremeValue object.

!syntax parameters /Postprocessors/SideExtremeValue

!syntax inputs /Postprocessors/SideExtremeValue

!syntax children /Postprocessors/SideExtremeValue

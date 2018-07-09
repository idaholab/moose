# SideAverageValue

!syntax description /Postprocessors/SideAverageValue

## Description

`SideAverageValue` computes the area- or volume-weighted average of the integral of the specified
variable. It may be used, for example, to calculate the average temperature over a side set.

!alert note title=SideAverageValue requires nonzero coordinate values
`SideAverageValue` is not suitable for use when computing the average integral value of a variable
when one of the coordinates in the sideset has a value of zero.  In those cases, such as when
computing the value of a variable on the centerline of an axisymmetric simulation, use
[AxisymmetricCenterlineAverageValue](/AxisymmetricCenterlineAverageValue.md)

## Example Input Syntax

!listing modules/tensor_mechanics/tutorials/basics/part_2.3.i block=Postprocessors

!syntax parameters /Postprocessors/SideAverageValue

!syntax inputs /Postprocessors/SideAverageValue

!syntax children /Postprocessors/SideAverageValue

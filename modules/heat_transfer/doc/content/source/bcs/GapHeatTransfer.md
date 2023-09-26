# GapHeatTransfer

!syntax description /BCs/GapHeatTransfer

## Description

GapHeatTransfer calculates the amount of heat transferred across unmeshed gaps between two different
blocks.

The `quadrature` option is generally recommended for most models. With this option, heat flux is
computed and applied as an integrated boundary condition at the quadrature points on both faces. Use
of the quadrature options generally gives smoother results, although there can be small differences
in the heat flux on the two surfaces.

It is also important to use the appropriate `gap_geometry_type` parameter (PLATE, CYLINDER, or
SPHERE) for the model geometry.

Two-dimensional Cartesian geometries are not restricted to be in or parallel to the X-Y coordinate plane.


## Example Input syntax

!listing modules/heat_conduction/test/tests/heat_conduction/2d_quadrature_gap_heat_transfer/nonmatching.i block=ThermalContact

!syntax parameters /BCs/GapHeatTransfer

!syntax inputs /BCs/GapHeatTransfer

!syntax children /BCs/GapHeatTransfer

# GapHeatTransfer

!syntax description /BCs/GapHeatTransfer

## Description

The `quadrature` option is recommended with second-order meshes. Also note that the
type of conductance used depends on the value of the `gap_geometry_type` parameter (PLATE,
CYLINDER, or SPHERE).

## Example Input syntax
!listing /modules/heat_conduction/test/tests/heat_conduction/2d_quadrature_gap_heat_transfer/nonmatching.i block=ThermalContact

!syntax parameters /BCs/GapHeatTransfer

!syntax inputs /BCs/GapHeatTransfer

!syntax children /BCs/GapHeatTransfer

# KokkosFunctionPathEllipsoidHeatSource

!if! function=hasCapability('kokkos')

This is the Kokkos version of [FunctionPathEllipsoidHeatSource](FunctionPathEllipsoidHeatSource.md). See the original document for details.

!alert note
The Kokkos version declares `volumetric_heat` as a non-AD material property.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_heat_source_materials.i start=[volumetric_heat] end=[] include-end=true

!syntax parameters /Materials/KokkosFunctionPathEllipsoidHeatSource

!syntax inputs /Materials/KokkosFunctionPathEllipsoidHeatSource

!syntax children /Materials/KokkosFunctionPathEllipsoidHeatSource

!if-end!

!else
!include kokkos/kokkos_warning.md

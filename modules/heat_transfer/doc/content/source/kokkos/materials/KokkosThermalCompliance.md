# KokkosThermalCompliance

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ThermalCompliance](ThermalCompliance.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_heat_source_materials.i start=[compliance] end=[] include-end=true

!syntax parameters /Materials/KokkosThermalCompliance

!syntax inputs /Materials/KokkosThermalCompliance

!syntax children /Materials/KokkosThermalCompliance

!if-end!

!else
!include kokkos/kokkos_warning.md

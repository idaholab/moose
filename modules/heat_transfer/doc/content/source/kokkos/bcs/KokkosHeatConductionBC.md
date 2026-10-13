# KokkosHeatConductionBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConductionBC](HeatConductionBC.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_temperature_dependent.i start=[conduction] end=[] include-end=true

!syntax parameters /BCs/KokkosHeatConductionBC

!syntax inputs /BCs/KokkosHeatConductionBC

!syntax children /BCs/KokkosHeatConductionBC

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosHeatConductionMaterial

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConductionMaterial](HeatConductionMaterial.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_conduction.i start=[conduction] end=[] include-end=true

!syntax parameters /Materials/KokkosHeatConductionMaterial

!syntax inputs /Materials/KokkosHeatConductionMaterial

!syntax children /Materials/KokkosHeatConductionMaterial

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosHeatSource

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatSource](HeatSource.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_heat_source_bar.i start=[heatsource] end=[] include-end=true

!syntax parameters /Kernels/KokkosHeatSource

!syntax inputs /Kernels/KokkosHeatSource

!syntax children /Kernels/KokkosHeatSource

!if-end!

!else
!include kokkos/kokkos_warning.md

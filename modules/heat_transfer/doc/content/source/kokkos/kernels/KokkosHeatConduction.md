# KokkosHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConduction](HeatConduction.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_conduction.i start=[heat_conduction] end=[] include-end=true

!syntax parameters /Kernels/KokkosHeatConduction

!syntax inputs /Kernels/KokkosHeatConduction

!syntax children /Kernels/KokkosHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

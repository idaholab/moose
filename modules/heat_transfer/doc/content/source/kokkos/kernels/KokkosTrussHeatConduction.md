# KokkosTrussHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [TrussHeatConduction](TrussHeatConduction.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_truss_heat_conduction.i start=[heat_conduction] end=[] include-end=true

!syntax parameters /Kernels/KokkosTrussHeatConduction

!syntax inputs /Kernels/KokkosTrussHeatConduction

!syntax children /Kernels/KokkosTrussHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

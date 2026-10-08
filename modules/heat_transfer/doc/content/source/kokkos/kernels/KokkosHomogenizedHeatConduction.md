# KokkosHomogenizedHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HomogenizedHeatConduction](HomogenizedHeatConduction.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_homogenization.i start=[heat_rhs_x] end=[] include-end=true

!syntax parameters /Kernels/KokkosHomogenizedHeatConduction

!syntax inputs /Kernels/KokkosHomogenizedHeatConduction

!syntax children /Kernels/KokkosHomogenizedHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosAnisoHomogenizedHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [AnisoHomogenizedHeatConduction](AnisoHomogenizedHeatConduction.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_homogenization_tensor.i start=[heat_rhs_x] end=[] include-end=true

!syntax parameters /Kernels/KokkosAnisoHomogenizedHeatConduction

!syntax inputs /Kernels/KokkosAnisoHomogenizedHeatConduction

!syntax children /Kernels/KokkosAnisoHomogenizedHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

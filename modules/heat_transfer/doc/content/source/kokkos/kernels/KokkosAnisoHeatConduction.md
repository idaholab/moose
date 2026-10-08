# KokkosAnisoHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [AnisoHeatConduction](AnisoHeatConduction.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_heat_conduction_ortho.i start=[heat] end=[] include-end=true

!syntax parameters /Kernels/KokkosAnisoHeatConduction

!syntax inputs /Kernels/KokkosAnisoHeatConduction

!syntax children /Kernels/KokkosAnisoHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

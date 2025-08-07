# KokkosHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConduction](HeatConduction.md). See the original document for details.

!alert note
Kokkos-MOOSE does not support displaced meshes yet. Therefore, the Kokkos version sets [!param](/KokkosKernels/KokkosHeatConduction/use_displaced_mesh) to false in contrast to the original version which sets [!param](/Kernels/HeatConduction/use_displaced_mesh) to true by default.

## Example Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_conduction.i start=[heat_conduction] end=[] include-end=true

!syntax parameters /KokkosKernels/KokkosHeatConduction

!syntax inputs /KokkosKernels/KokkosHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

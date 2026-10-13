# KokkosHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConduction](HeatConduction.md). See the original document for details.

!alert note
The Kokkos version defaults [!param](/Kernels/KokkosHeatConduction/use_displaced_mesh) to false, whereas the original version integrates over the displaced mesh by default. With `use_displaced_mesh = true` the term is posed on the current configuration: the kernel integrates on the reference mesh with the deformation gradient composed into its geometry, and the framework assembles its Jacobian with respect to the displacements.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_conduction.i start=[heat_conduction] end=[] include-end=true

!syntax parameters /Kernels/KokkosHeatConduction

!syntax inputs /Kernels/KokkosHeatConduction

!syntax children /Kernels/KokkosHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

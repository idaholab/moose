# KokkosHeatConductionTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConductionTimeDerivative](HeatConductionTimeDerivative.md). See the original document for details.

!alert note
Kokkos-MOOSE does not support displaced meshes yet. Therefore, the Kokkos version sets [!param](/Kernels/KokkosHeatConductionTimeDerivative/use_displaced_mesh) to false in contrast to the original version which sets [!param](/Kernels/HeatConductionTimeDerivative/use_displaced_mesh) to true by default.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_transient_conduction.i start=[heat_conduction_time] end=[] include-end=true

!syntax parameters /Kernels/KokkosHeatConductionTimeDerivative

!syntax inputs /Kernels/KokkosHeatConductionTimeDerivative

!syntax children /Kernels/KokkosHeatConductionTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md

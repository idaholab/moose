# KokkosHeatCapacityConductionTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatCapacityConductionTimeDerivative](HeatCapacityConductionTimeDerivative.md). See the original document for details.

!alert note
Kokkos-MOOSE does not support displaced meshes yet. Therefore, the Kokkos version sets [!param](/Kernels/KokkosHeatCapacityConductionTimeDerivative/use_displaced_mesh) to false in contrast to the original version which sets [!param](/Kernels/HeatCapacityConductionTimeDerivative/use_displaced_mesh) to true by default.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_temperature_dependent.i start=[time] end=[] include-end=true

!syntax parameters /Kernels/KokkosHeatCapacityConductionTimeDerivative

!syntax inputs /Kernels/KokkosHeatCapacityConductionTimeDerivative

!syntax children /Kernels/KokkosHeatCapacityConductionTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md

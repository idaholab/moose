# KokkosSpecificHeatConductionTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SpecificHeatConductionTimeDerivative](SpecificHeatConductionTimeDerivative.md). See the original document for details.

!alert note
Kokkos-MOOSE does not support displaced meshes yet. Therefore, the Kokkos version sets [!param](/Kernels/KokkosSpecificHeatConductionTimeDerivative/use_displaced_mesh) to false in contrast to the original version which sets [!param](/Kernels/SpecificHeatConductionTimeDerivative/use_displaced_mesh) to true by default.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_specific_heat_transient.i start=[ie] end=[] include-end=true

!syntax parameters /Kernels/KokkosSpecificHeatConductionTimeDerivative

!syntax inputs /Kernels/KokkosSpecificHeatConductionTimeDerivative

!syntax children /Kernels/KokkosSpecificHeatConductionTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md

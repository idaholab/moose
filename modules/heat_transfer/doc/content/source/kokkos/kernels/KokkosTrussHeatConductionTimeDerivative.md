# KokkosTrussHeatConductionTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [TrussHeatConductionTimeDerivative](TrussHeatConductionTimeDerivative.md). See the original document for details.

!alert note
Kokkos-MOOSE does not support displaced meshes yet. Therefore, the Kokkos version sets [!param](/Kernels/KokkosTrussHeatConductionTimeDerivative/use_displaced_mesh) to false in contrast to the original version which sets [!param](/Kernels/TrussHeatConductionTimeDerivative/use_displaced_mesh) to true by default.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_truss_heat_conduction.i start=[time_derivative] end=[] include-end=true

!syntax parameters /Kernels/KokkosTrussHeatConductionTimeDerivative

!syntax inputs /Kernels/KokkosTrussHeatConductionTimeDerivative

!syntax children /Kernels/KokkosTrussHeatConductionTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md

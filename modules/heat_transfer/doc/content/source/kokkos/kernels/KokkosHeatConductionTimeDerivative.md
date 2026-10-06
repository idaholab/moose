# KokkosHeatConductionTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConductionTimeDerivative](HeatConductionTimeDerivative.md). See the original document for details.

!alert note
Kokkos-MOOSE objects are formulated on the reference configuration and do not accept `use_displaced_mesh`. The original version integrates over the displaced mesh by default. Here the term is integrated over the reference configuration, so [!param](/Kernels/KokkosHeatConductionTimeDerivative/density) is the reference density. Conservation of mass, $\rho\,dv = \rho_0\,dV$, makes this the same term as the current-configuration integral with the current density.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_transient_conduction.i start=[heat_conduction_time] end=[] include-end=true

!syntax parameters /Kernels/KokkosHeatConductionTimeDerivative

!syntax inputs /Kernels/KokkosHeatConductionTimeDerivative

!syntax children /Kernels/KokkosHeatConductionTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosHeatConductionMaterial

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConductionMaterial](HeatConductionMaterial.md). See the original document for details.

!alert note
Kokkos-MOOSE does not support functions yet. Therefore, [!param](/Materials/HeatConductionMaterial/thermal_conductivity_temperature_function) and [!param](/Materials/HeatConductionMaterial/specific_heat_temperature_function) in the original [HeatConductionMaterial.md] are not available in the Kokkos version.

## Example Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_conduction.i start=[conduction] end=[] include-end=true

!syntax parameters /Materials/KokkosHeatConductionMaterial

!syntax inputs /Materials/KokkosHeatConductionMaterial

!syntax children /Materials/KokkosHeatConductionMaterial

!if-end!

!else
!include kokkos/kokkos_warning.md

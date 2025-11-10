# KokkosRadiativeHeatFluxBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [RadiativeHeatFluxBC](RadiativeHeatFluxBC.md). See the original document for details.

!alert note
Kokkos-MOOSE does not support functions yet. Therefore, [!param](/BCs/RadiativeHeatFluxBC/view_factor) in the original version is not available in the Kokkos version.

## Example Syntax

!listing thermal_hydraulics/test/tests/kokkos/kokkos_radiation_heat_flux_bc.i start=[bc] end=[] include-end=true

!syntax parameters /BCs/KokkosRadiativeHeatFluxBC

!syntax inputs /BCs/KokkosRadiativeHeatFluxBC

!syntax children /BCs/KokkosRadiativeHeatFluxBC

!if-end!

!else
!include kokkos/kokkos_warning.md

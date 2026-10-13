# KokkosRadiativeHeatFluxBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of `RadiativeHeatFluxBC` of the thermal hydraulics module. See the [original document](https://mooseframework.inl.gov/source/bcs/RadiativeHeatFluxBC.html) for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_radiation_heat_flux_bc.i start=[bc] end=[] include-end=true

!syntax parameters /BCs/KokkosRadiativeHeatFluxBC

!syntax inputs /BCs/KokkosRadiativeHeatFluxBC

!syntax children /BCs/KokkosRadiativeHeatFluxBC

!if-end!

!else
!include kokkos/kokkos_warning.md

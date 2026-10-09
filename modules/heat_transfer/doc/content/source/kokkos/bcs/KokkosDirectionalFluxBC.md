# KokkosDirectionalFluxBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [DirectionalFluxBC](DirectionalFluxBC.md). See the original document for details.

!alert note
The self shadowing calculation must be provided by a [KokkosSelfShadowSideUserObject](KokkosSelfShadowSideUserObject.md) through [!param](/BCs/KokkosDirectionalFluxBC/self_shadow_uo).

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_directional_flux_2d.i start=[flux_v] end=[] include-end=true

!syntax parameters /BCs/KokkosDirectionalFluxBC

!syntax inputs /BCs/KokkosDirectionalFluxBC

!syntax children /BCs/KokkosDirectionalFluxBC

!if-end!

!else
!include kokkos/kokkos_warning.md

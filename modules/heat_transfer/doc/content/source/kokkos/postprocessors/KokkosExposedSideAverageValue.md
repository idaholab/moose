# KokkosExposedSideAverageValue

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ExposedSideAverageValue](ExposedSideAverageValue.md). See the original document for details.

!alert note
The illumination state must be provided by a [KokkosSelfShadowSideUserObject](KokkosSelfShadowSideUserObject.md) through [!param](/Postprocessors/KokkosExposedSideAverageValue/self_shadow_uo).

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_directional_flux_2d.i start=[ave_v_exposed] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosExposedSideAverageValue

!syntax inputs /Postprocessors/KokkosExposedSideAverageValue

!syntax children /Postprocessors/KokkosExposedSideAverageValue

!if-end!

!else
!include kokkos/kokkos_warning.md

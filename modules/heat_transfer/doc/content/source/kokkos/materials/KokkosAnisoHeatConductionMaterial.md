# KokkosAnisoHeatConductionMaterial

!if! function=hasCapability('kokkos')

This is the Kokkos version of [AnisoHeatConductionMaterial](AnisoHeatConductionMaterial.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_heat_conduction_ortho.i start=[heat] end=[] include-end=true

!syntax parameters /Materials/KokkosAnisoHeatConductionMaterial

!syntax inputs /Materials/KokkosAnisoHeatConductionMaterial

!syntax children /Materials/KokkosAnisoHeatConductionMaterial

!if-end!

!else
!include kokkos/kokkos_warning.md

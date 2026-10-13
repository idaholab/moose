# KokkosInfiniteCylinderRadiativeBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [InfiniteCylinderRadiativeBC](InfiniteCylinderRadiativeBC.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_radiative_bc_cyl.i start=[radiative_bc] end=[] include-end=true

!syntax parameters /BCs/KokkosInfiniteCylinderRadiativeBC

!syntax inputs /BCs/KokkosInfiniteCylinderRadiativeBC

!syntax children /BCs/KokkosInfiniteCylinderRadiativeBC

!if-end!

!else
!include kokkos/kokkos_warning.md

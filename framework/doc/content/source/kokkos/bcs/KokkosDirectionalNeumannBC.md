# KokkosDirectionalNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [DirectionalNeumannBC](DirectionalNeumannBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/directional_neumann/kokkos_2d_directional_neumann_bc_test.i start=[top] end=[] include-end=true

!syntax parameters /BCs/KokkosDirectionalNeumannBC

!syntax inputs /BCs/KokkosDirectionalNeumannBC

!syntax children /BCs/KokkosDirectionalNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md

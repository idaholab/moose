# KokkosArrayDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ArrayDirichletBC](ArrayDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/array/kokkos_array_bcs.i start=[left] end=[] include-end=true

!syntax parameters /BCs/KokkosArrayDirichletBC

!syntax inputs /BCs/KokkosArrayDirichletBC

!syntax children /BCs/KokkosArrayDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

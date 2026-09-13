# KokkosArrayNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ArrayNeumannBC](ArrayNeumannBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/array/kokkos_array_bcs.i start=[right] end=[] include-end=true

!syntax parameters /BCs/KokkosArrayNeumannBC

!syntax inputs /BCs/KokkosArrayNeumannBC

!syntax children /BCs/KokkosArrayNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md

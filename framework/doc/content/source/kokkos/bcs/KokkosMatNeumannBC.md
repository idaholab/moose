# KokkosMatNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [MatNeumannBC](MatNeumannBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/materials/parsed/kokkos_parsed_material.i start=[top] end=[] include-end=true

!syntax parameters /BCs/KokkosMatNeumannBC

!syntax inputs /BCs/KokkosMatNeumannBC

!syntax children /BCs/KokkosMatNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md

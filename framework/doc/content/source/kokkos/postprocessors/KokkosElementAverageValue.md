# KokkosElementAverageValue

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementAverageValue](ElementAverageValue.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/materials/stateful_prop/kokkos_stateful_prop_test.i start=[integral] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosElementAverageValue

!syntax inputs /Postprocessors/KokkosElementAverageValue

!syntax children /Postprocessors/KokkosElementAverageValue

!if-end!

!else
!include kokkos/kokkos_warning.md

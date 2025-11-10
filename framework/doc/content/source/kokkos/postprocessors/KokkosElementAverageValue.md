# KokkosElementAverageValue

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementAverageValue](ElementAverageValue.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/element_integral/kokkos_element_block_average_test.i block=Postprocessors

!syntax parameters /Postprocessors/KokkosElementAverageValue

!syntax inputs /Postprocessors/KokkosElementAverageValue

!syntax children /Postprocessors/KokkosElementAverageValue

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosElementExtremeValue

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementExtremeValue](ElementExtremeValue.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/element_extreme_value/kokkos_element_extreme_value.i block=Postprocessors

!syntax parameters /Postprocessors/KokkosElementExtremeValue

!syntax inputs /Postprocessors/KokkosElementExtremeValue

!syntax children /Postprocessors/KokkosElementExtremeValue

!if-end!

!else
!include kokkos/kokkos_warning.md

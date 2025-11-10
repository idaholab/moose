# KokkosElementIntegralVariablePostprocessor

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementIntegralVariablePostprocessor](ElementIntegralVariablePostprocessor.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/element_integral/kokkos_element_integral_test.i start=[integral] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosElementIntegralVariablePostprocessor

!syntax inputs /Postprocessors/KokkosElementIntegralVariablePostprocessor

!syntax children /Postprocessors/KokkosElementIntegralVariablePostprocessor

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosElementIntegralVariablePostprocessor

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementIntegralVariablePostprocessor](ElementIntegralVariablePostprocessor.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/functions/piecewise_constant/kokkos_piecewise_constant.i start=[coef] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosElementIntegralVariablePostprocessor

!syntax inputs /Postprocessors/KokkosElementIntegralVariablePostprocessor

!syntax children /Postprocessors/KokkosElementIntegralVariablePostprocessor

!if-end!

!else
!include kokkos/kokkos_warning.md

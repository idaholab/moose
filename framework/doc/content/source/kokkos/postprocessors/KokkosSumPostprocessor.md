# KokkosSumPostprocessor

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SumPostprocessor](SumPostprocessor.md). See the original document for details.

!alert note
The functionality of this object is identical to the original [SumPostprocessor](SumPostprocessor.md) except that this object is a [Kokkos-MOOSE general user object](syntax/KokkosUserObjects/index.md) and has automatic dependency resolution with Kokkos-MOOSE user objects.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/dependency/kokkos_general_uo_dependency_test.i start=[kokkos_sum] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosSumPostprocessor

!syntax inputs /Postprocessors/KokkosSumPostprocessor

!syntax children /Postprocessors/KokkosSumPostprocessor

!if-end!

!else
!include kokkos/kokkos_warning.md

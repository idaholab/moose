# KokkosNodalExtremeValue

!if! function=hasCapability('kokkos')

This is the Kokkos version of [NodalExtremeValue](NodalExtremeValue.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/nodal_extreme_value/kokkos_nodal_max_value_test.i start=[max_nodal_val] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosNodalExtremeValue

!syntax inputs /Postprocessors/KokkosNodalExtremeValue

!syntax children /Postprocessors/KokkosNodalExtremeValue

!if-end!

!else
!include kokkos/kokkos_warning.md

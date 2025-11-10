# KokkosElementL2Norm

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementL2Norm](ElementL2Norm.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/element_l2_norm/kokkos_element_l2_norm.i start=[L2_norm] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosElementL2Norm

!syntax inputs /Postprocessors/KokkosElementL2Norm

!syntax children /Postprocessors/KokkosElementL2Norm

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosNodalL2Norm

!if! function=hasCapability('kokkos')

This is the Kokkos version of [NodalL2Norm](NodalL2Norm.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/nodal_l2_norm/kokkos_nodal_l2_norm.i start=[nodal_L2_norm] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosNodalL2Norm

!syntax inputs /Postprocessors/KokkosNodalL2Norm

!syntax children /Postprocessors/KokkosNodalL2Norm

!if-end!

!else
!include kokkos/kokkos_warning.md

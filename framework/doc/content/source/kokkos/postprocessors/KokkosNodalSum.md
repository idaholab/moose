# KokkosNodalSum

!if! function=hasCapability('kokkos')

This is the Kokkos version of [NodalSum](NodalSum.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/nodal_sum/kokkos_nodal_sum.i start=[nodal_sum] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosNodalSum

!syntax inputs /Postprocessors/KokkosNodalSum

!syntax children /Postprocessors/KokkosNodalSum

!if-end!

!else
!include kokkos/kokkos_warning.md

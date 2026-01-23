# KokkosNodalMaxValueId

!if! function=hasCapability('kokkos')

This is the Kokkos version of [NodalMaxValueId](NodalMaxValueId.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/nodal_extreme_value/kokkos_nodal_extreme_pps_test.i start=[max_node_id] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosNodalMaxValueId

!syntax inputs /Postprocessors/KokkosNodalMaxValueId

!syntax children /Postprocessors/KokkosNodalMaxValueId

!if-end!

!else
!include kokkos/kokkos_warning.md

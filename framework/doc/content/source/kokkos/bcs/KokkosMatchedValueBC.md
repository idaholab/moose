# KokkosMatchedValueBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [MatchedValueBC](MatchedValueBC.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/bcs/matched_value_bc/kokkos_matched_value_bc_test.i start=[left_u] end=[] include-end=true

!syntax parameters /KokkosBCs/KokkosMatchedValueBC

!syntax inputs /KokkosBCs/KokkosMatchedValueBC

!if-end!

!else
!include kokkos/kokkos_warning.md

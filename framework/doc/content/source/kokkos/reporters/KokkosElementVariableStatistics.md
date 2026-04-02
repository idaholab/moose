# KokkosElementVariableStatistics

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementVariableStatistics](ElementVariableStatistics.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/reporters/element_reporter/kokkos_elem_stats.i start=[elem_stats] end=[] include-end=true

!syntax parameters /Reporters/KokkosElementVariableStatistics

!syntax inputs /Reporters/KokkosElementVariableStatistics

!syntax children /Reporters/KokkosElementVariableStatistics

!if-end!

!else
!include kokkos/kokkos_warning.md

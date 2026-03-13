# KokkosNodalVariableStatistics

!if! function=hasCapability('kokkos')

This is the Kokkos version of [NodalVariableStatistics](NodalVariableStatistics.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/reporters/nodal_reporter/kokkos_nodal_stats.i start=[nodal_stats] end=[] include-end=true

!syntax parameters /Reporters/KokkosNodalVariableStatistics

!syntax inputs /Reporters/KokkosNodalVariableStatistics

!syntax children /Reporters/KokkosNodalVariableStatistics

!if-end!

!else
!include kokkos/kokkos_warning.md

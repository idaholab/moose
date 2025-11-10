# KokkosVacuumBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [VacuumBC](VacuumBC.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/bcs/misc_bcs/kokkos_vacuum_bc_test.i start=[top] end=[] include-end=true

!syntax parameters /BCs/KokkosVacuumBC

!syntax inputs /BCs/KokkosVacuumBC

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [DirichletBC](DirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/coupled_dirichlet_bc/kokkos_coupled_dirichlet_bc.i start=[left_u] end=[] include-end=true

!syntax parameters /KokkosBCs/KokkosDirichletBC

!syntax inputs /KokkosBCs/KokkosDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

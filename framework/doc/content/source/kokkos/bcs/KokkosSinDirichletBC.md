# KokkosSinDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SinDirichletBC](SinDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/sin_dirichlet/kokkos_sin_dirichlet.i start=[left] end=[] include-end=true

!syntax parameters /BCs/KokkosSinDirichletBC

!syntax inputs /BCs/KokkosSinDirichletBC

!syntax children /BCs/KokkosSinDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

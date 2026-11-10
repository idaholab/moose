# KokkosFunctionDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [FunctionDirichletBC](FunctionDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/function_dirichlet/kokkos_function_dirichlet.i start=[all] end=[] include-end=true

!syntax parameters /BCs/KokkosFunctionDirichletBC

!syntax inputs /BCs/KokkosFunctionDirichletBC

!syntax children /BCs/KokkosFunctionDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

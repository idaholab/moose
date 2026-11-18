# KokkosVectorFunctionDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [VectorFunctionDirichletBC](VectorFunctionDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/vector_function_dirichlet/kokkos_vector_function_dirichlet.i start=[left_right] end=[] include-end=true

!syntax parameters /BCs/KokkosVectorFunctionDirichletBC

!syntax inputs /BCs/KokkosVectorFunctionDirichletBC

!syntax children /BCs/KokkosVectorFunctionDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

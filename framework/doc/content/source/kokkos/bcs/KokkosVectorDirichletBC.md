# KokkosVectorDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [VectorDirichletBC](VectorDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/vector_fe/kokkos_coupled_vector_gradient.i start=[left_u] end=[] include-end=true

!syntax parameters /BCs/KokkosVectorDirichletBC

!syntax inputs /BCs/KokkosVectorDirichletBC

!syntax children /BCs/KokkosVectorDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

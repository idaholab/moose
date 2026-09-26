# KokkosVectorFunctionDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [VectorFunctionDirichletBC](VectorFunctionDirichletBC.md), taking
one function per component through [!param](/BCs/KokkosVectorFunctionDirichletBC/function_x),
[!param](/BCs/KokkosVectorFunctionDirichletBC/function_y) and
[!param](/BCs/KokkosVectorFunctionDirichletBC/function_z). A component whose function is omitted is
held at zero. Each function is evaluated on the device, so each must be a
[KokkosParsedFunction](KokkosParsedFunction.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/vector_fe/kokkos_lagrange_vec.i block=BCs

!syntax parameters /BCs/KokkosVectorFunctionDirichletBC

!syntax inputs /BCs/KokkosVectorFunctionDirichletBC

!syntax children /BCs/KokkosVectorFunctionDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

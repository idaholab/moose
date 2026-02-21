# KokkosEigenDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [EigenDirichletBC](EigenDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/problems/eigen_problem/kokkos_eigenvalue_test.i start=[eigen] end=[] include-end=true

!syntax parameters /BCs/KokkosEigenDirichletBC

!syntax inputs /BCs/KokkosEigenDirichletBC

!syntax children /BCs/KokkosEigenDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

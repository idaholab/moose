# KokkosADFunctionDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ADFunctionDirichletBC](ADFunctionDirichletBC.md). See the original
document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/hierarchic_dirichlet_mms/hierarchic_dirichlet_mms.i start=[all] end=[] include-end=true

!syntax parameters /BCs/KokkosADFunctionDirichletBC

!syntax inputs /BCs/KokkosADFunctionDirichletBC

!syntax children /BCs/KokkosADFunctionDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

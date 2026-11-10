# KokkosPostprocessorDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PostprocessorDirichletBC](PostprocessorDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/pp_dirichlet/kokkos_pp_dirichlet.i start=[right] end=[] include-end=true

!syntax parameters /BCs/KokkosPostprocessorDirichletBC

!syntax inputs /BCs/KokkosPostprocessorDirichletBC

!syntax children /BCs/KokkosPostprocessorDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md

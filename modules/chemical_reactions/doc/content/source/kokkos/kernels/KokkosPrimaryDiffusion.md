# KokkosPrimaryDiffusion

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PrimaryDiffusion](PrimaryDiffusion.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[a_diff] end=[] include-end=true

!syntax parameters /Kernels/KokkosPrimaryDiffusion

!syntax inputs /Kernels/KokkosPrimaryDiffusion

!syntax children /Kernels/KokkosPrimaryDiffusion

!if-end!

!else
!include kokkos/kokkos_warning.md

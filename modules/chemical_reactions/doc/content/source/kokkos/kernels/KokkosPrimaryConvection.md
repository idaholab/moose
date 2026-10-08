# KokkosPrimaryConvection

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PrimaryConvection](PrimaryConvection.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[a_conv] end=[] include-end=true

!syntax parameters /Kernels/KokkosPrimaryConvection

!syntax inputs /Kernels/KokkosPrimaryConvection

!syntax children /Kernels/KokkosPrimaryConvection

!if-end!

!else
!include kokkos/kokkos_warning.md

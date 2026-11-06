# KokkosPrimaryTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PrimaryTimeDerivative](PrimaryTimeDerivative.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[a_ie] end=[] include-end=true

!syntax parameters /Kernels/KokkosPrimaryTimeDerivative

!syntax inputs /Kernels/KokkosPrimaryTimeDerivative

!syntax children /Kernels/KokkosPrimaryTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosCoupledBEEquilibriumSub

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledBEEquilibriumSub](CoupledBEEquilibriumSub.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[a1eq] end=[] include-end=true

!syntax parameters /Kernels/KokkosCoupledBEEquilibriumSub

!syntax inputs /Kernels/KokkosCoupledBEEquilibriumSub

!syntax children /Kernels/KokkosCoupledBEEquilibriumSub

!if-end!

!else
!include kokkos/kokkos_warning.md

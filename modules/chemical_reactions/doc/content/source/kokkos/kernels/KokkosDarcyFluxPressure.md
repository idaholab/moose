# KokkosDarcyFluxPressure

!if! function=hasCapability('kokkos')

This is the Kokkos version of [DarcyFluxPressure](DarcyFluxPressure.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_equilibrium_jacobian.i start=[diff] end=[] include-end=true

!syntax parameters /Kernels/KokkosDarcyFluxPressure

!syntax inputs /Kernels/KokkosDarcyFluxPressure

!syntax children /Kernels/KokkosDarcyFluxPressure

!if-end!

!else
!include kokkos/kokkos_warning.md

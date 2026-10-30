# KokkosArrayCoupledForce

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ArrayCoupledForce](ArrayCoupledForce.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/array_coupled_force/kokkos_array_coupled_force.i block=Kernels

!syntax parameters /Kernels/KokkosArrayCoupledForce

!syntax inputs /Kernels/KokkosArrayCoupledForce

!syntax children /Kernels/KokkosArrayCoupledForce

!if-end!

!else
!include kokkos/kokkos_warning.md

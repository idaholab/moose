# KokkosCoupledForce

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledForce](CoupledForce.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/bcs/coupled_dirichlet_bc/kokkos_coupled_dirichlet_bc.i start=[coupled_force_u] end=[] include-end=true

!syntax parameters /KokkosKernels/KokkosCoupledForce

!syntax inputs /KokkosKernels/KokkosCoupledForce

!if-end!

!else
!include kokkos/kokkos_warning.md

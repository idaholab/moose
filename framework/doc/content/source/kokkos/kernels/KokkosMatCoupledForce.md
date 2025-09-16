# KokkosMatCoupledForce

!if! function=hasCapability('kokkos')

This is the Kokkos version of [MatCoupledForce](MatCoupledForce.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/kernels/material_coupled_force/kokkos_material_coupled_force.i start=[coupled] end=[] include-end=true

!syntax parameters /KokkosKernels/KokkosMatCoupledForce

!syntax inputs /KokkosKernels/KokkosMatCoupledForce

!if-end!

!else
!include kokkos/kokkos_warning.md

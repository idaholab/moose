# KokkosCoupledBEKinetic

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledBEKinetic](CoupledBEKinetic.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_kinetic_rate.i start=[a0_r] end=[] include-end=true

!syntax parameters /Kernels/KokkosCoupledBEKinetic

!syntax inputs /Kernels/KokkosCoupledBEKinetic

!syntax children /Kernels/KokkosCoupledBEKinetic

!if-end!

!else
!include kokkos/kokkos_warning.md

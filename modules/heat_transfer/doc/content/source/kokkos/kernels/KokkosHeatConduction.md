# KokkosHeatConduction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HeatConduction](HeatConduction.md). See the original document for details.

!alert note
Kokkos-MOOSE objects are formulated on the reference configuration and do not accept `use_displaced_mesh`. The original version integrates over the displaced mesh by default. To pose the conduction term on the deformed configuration here, couple [!param](/Kernels/KokkosHeatConduction/displacements). The kernel then pulls the term back to the reference configuration with the deformation gradient $\mathbf{F} = \mathbf{I} + \nabla_X \mathbf{u}$ and its determinant $J$, $(k J \mathbf{F}^{-1}\mathbf{F}^{-T}\nabla_X T, \nabla_X \psi_i)$, and contributes the off-diagonal Jacobian with respect to the displacements.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_conduction.i start=[heat_conduction] end=[] include-end=true

!syntax parameters /Kernels/KokkosHeatConduction

!syntax inputs /Kernels/KokkosHeatConduction

!syntax children /Kernels/KokkosHeatConduction

!if-end!

!else
!include kokkos/kokkos_warning.md

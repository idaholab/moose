# KokkosADMatDiffusion

!if! function=hasCapability('kokkos')

!syntax description /Kernels/KokkosADMatDiffusion

This is the Kokkos version of `ADMatDiffusion`, described in [MatDiffusion](MatDiffusion.md). The diffusivity is a pointwise
automatic differentiation (AD) material property declared by a Kokkos AD material, which stores
the derivatives of the property with respect to the inputs of the material. The derivatives with
respect to inputs coupled to the kernel variable, and with respect to the displacements when the
material is posed on the current configuration, enter the Jacobian of this kernel.

## Example Input Syntax

!listing test/tests/kokkos/kernels/ad_mat_diffusion/kokkos_ad_mat_diffusion_jacobian.i block=Kernels

!syntax parameters /Kernels/KokkosADMatDiffusion

!syntax inputs /Kernels/KokkosADMatDiffusion

!syntax children /Kernels/KokkosADMatDiffusion

!if-end!

!else
!include kokkos/kokkos_warning.md

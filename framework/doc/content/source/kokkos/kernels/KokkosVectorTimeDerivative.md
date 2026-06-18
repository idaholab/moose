# KokkosVectorTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [VectorTimeDerivative](VectorTimeDerivative.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/transient_vector_diffusion/kokkos_transient_vector_diffusion.i start=[time] end=[] include-end=true

!syntax parameters /Kernels/KokkosVectorTimeDerivative

!syntax inputs /Kernels/KokkosVectorTimeDerivative

!syntax children /Kernels/KokkosVectorTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md

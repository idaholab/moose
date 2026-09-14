# KokkosBodyForce

!if! function=hasCapability('kokkos')

This is the Kokkos version of [BodyForce](BodyForce.md). See the original document for details.

!alert note
[!param](/Kernels/KokkosBodyForce/function) must name a [KokkosParsedFunction](KokkosParsedFunction.md), which is
also what a numeric value given to that parameter is built as. Kokkos-MOOSE retrieves a function in its own concrete
type, so a function of another type cannot be substituted here.

## Example Input Syntax

!listing test/tests/kokkos/kernels/2d_diffusion/kokkos_2d_diffusion_bodyforce_test.i start=[bf] end=[] include-end=true

!syntax parameters /Kernels/KokkosBodyForce

!syntax inputs /Kernels/KokkosBodyForce

!syntax children /Kernels/KokkosBodyForce

!if-end!

!else
!include kokkos/kokkos_warning.md

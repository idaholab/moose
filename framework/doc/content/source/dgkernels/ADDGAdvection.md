# ADDGAdvection

!syntax description /DGKernels/ADDGAdvection

This is the automatic differentiation analog of [DGConvection.md] but with a
couple distinctions:

- The [!param](/DGKernels/ADDGAdvection/velocity) parameter is a material
  property instead of a constant. This allows this object's use in a
  simulation where the velocity is being solved for
  and/or changing spatially. Additionally, the use of a material property as
  opposed to a coupled variable allows more straightforward
  propagation of derivatives for automatic differentiation.
- A [!param](/DGKernels/ADDGAdvection/advected_quantity) parameter is
  available which allows for advecting different quantities than the `variable`
  this object is acting upon

## Example input syntax

In this example, a field `u` is advected from a boundary condition on its left to the right boundary
with a `1 0 0` velocity. In addition to advection, the simulation is governed by
loss of `u` through diffusion out of the top
and bottom boundaries of the domain.

!listing test/tests/dgkernels/passive-scalar-channel-flow/test.i block=DGKernels

!syntax parameters /DGKernels/ADDGAdvection

!syntax inputs /DGKernels/ADDGAdvection

!syntax children /DGKernels/ADDGAdvection

# ADConservativeAdvection

## Description

The `ADConservativeAdvection` kernel implements the same advection term as
[ConservativeAdvection.md]. A few differences from that object are:

- The [!param](/Kernels/ADConservativeAdvection/velocity) parameter is a material
  property instead of a coupled variable. This allows more straightforward
  propagation of derivatives for automatic differentiation
- No upwinding option is currently implemented. In that vein this object may be
  best used within a discontinuous Galerkin scheme with [ADDGAdvection.md].
- A [!param](/Kernels/ADConservativeAdvection/advected_quantity) parameter is
  available which allows for advecting different quantities than the `variable`
  this object is acting upon

!syntax parameters /Kernels/ADConservativeAdvection

!syntax inputs /Kernels/ADConservativeAdvection

!syntax children /Kernels/ADConservativeAdvection

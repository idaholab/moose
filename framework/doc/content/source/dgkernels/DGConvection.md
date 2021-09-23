# DGConvection

!syntax description /DGKernels/DGConvection

The velocity vector is a constant, so this may not be used for solving for the velocity in fluid
simulations for example. This kernel is only for advecting a field with a known velocity.
The first order upwind scheme used is diffusive and will only provide first order mesh convergence.

More information about the discontinuous Galerkin method may be found in the
[DGKernels syntax page](syntax/DGKernels/index.md).

## Example input syntax

In this example, a field `u` is advected from a boundary condition on its left to the right boundary
with a `1 0 0` velocity using the `ADDGConvection`, the version of this kernel with automatic differentiation.

!listing test/tests/dgkernels/ad_dg_convection/ad_dg_convection.i block=DGKernels

!syntax parameters /DGKernels/DGConvection

!syntax inputs /DGKernels/DGConvection

!syntax children /DGKernels/DGConvection

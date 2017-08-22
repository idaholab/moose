
# WeakGradientBC
!syntax description /BCs/WeakGradientBC

## Description
`WeakGradientBC` is an `IntegratedBC` which is appropriate for use with the
boundary terms arising from the [`Diffusion`](/framework/Diffusion.md) [`Kernel`](/Kernels/index.md). `WeakGradientBC`
does not "enforce" a boundary condition per-se (see,
e.g. [`DirichletBC`](/framework/DirichletBC.md), [`NeumannBC`](/framework/NeumannBC.md), and related classes for that).
Instead, this class is responsible for computing the residual (and
Jacobian) contributions due to the boundary contribution arising from
integration by parts on the [`Diffusion`](/framework/Diffusion.md) [`Kernel`](/Kernels/index.md).

This class computes a residual contribution identical to that of the
[DiffusionFluxBC](/BCs/framework/DiffusionFluxBC.md) class. Please see
that class' documentation for more detailed information.

## Example Input Syntax
!listing test/tests/bcs/misc_bcs/weak_gradient_bc_test.i start=[./top] end=[../] include-end=true

!syntax parameters /BCs/WeakGradientBC

!syntax inputs /BCs/WeakGradientBC

!syntax children /BCs/WeakGradientBC

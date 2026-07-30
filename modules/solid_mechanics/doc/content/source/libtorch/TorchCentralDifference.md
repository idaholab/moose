# TorchCentralDifference

!if! function=hasLibtorch()

This object functions in much the same way as [ExplicitMixedOrder](ExplicitMixedOrder.md), but completely skips the regular element/node loops when evaluating the right-hand-side residual. It is designed to drive batched libtorch finite-element kernels (and, together with a [NEML2](NEML2/index.md) model, to perform the material model evaluation and residual assembly).

## Implementation details

This object caches the finite element context using the [TorchAssembly](TorchAssembly.md) and [TorchFEInterpolation](TorchFEInterpolation.md) objects. The solution is re-interpolated onto the finite element function space after each explicit solution update.

In addition, this object zeros out the algebraic element/node ranges before evaluating the residual. Therefore, regular MOOSE kernels don't work with this time integrator.

### Limitations

- Designed for explicit solves only; no Jacobian contributions are produced by the Torch FEM kernels in this workflow.
- Requires the Torch assembly/interpolation objects to have compatible element types within each block (one per element type/order when mixing).

## Syntax

!syntax parameters /Executioner/TimeIntegrators/TorchCentralDifference

## Example input files

!syntax inputs /Executioner/TimeIntegrators/TorchCentralDifference

!if-end!

!else

!include libtorch/libtorch_warning.md

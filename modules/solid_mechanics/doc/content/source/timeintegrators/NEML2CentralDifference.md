# NEML2CentralDifference

!if! function=hasCapability('neml2')

This object functions in much the similar way as [ExplicitMixedOrder](ExplicitMixedOrder.md), but completely skips the regular element/node loops when evaluating the right-hand-side residual. This object is designed to interface with [NEML2](NEML2/index.md) to perform the material model evaluation and residual assembly.

## Implementation details

This object caches the finite element context using the [NEML2FEAssembly](NEML2Assembly.md) and the [NEML2FEInterpolation](NEML2FEInterpolation.md) objects. The solution is re-interpolated onto the finite element function space after each explicit solution update.

In addition, this object zeros out the algebraic element/node ranges before evaluating the residual. Therefore, regular MOOSE kernels don't work with this time integrator.

## Syntax

!syntax parameters /Executioner/TimeIntegrators/NEML2CentralDifference

## Example input files

!syntax inputs /Executioner/TimeIntegrators/NEML2CentralDifference

!if-end!

!else

!include neml2/neml2_warning.md

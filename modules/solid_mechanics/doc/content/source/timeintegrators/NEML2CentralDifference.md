# NEML2CentralDifference

!if! function=hasCapability('neml2')

!syntax description /Executioner/TimeIntegrators/NEML2CentralDifference

## Description

This object functions similarly to [ExplicitMixedOrder](ExplicitMixedOrder.md),
but bypasses the regular domain-element residual loop after the NEML2 finite
element caches are initialized. It interfaces with [NEML2](NEML2/index.md) for
material model evaluation and residual assembly.

## Implementation Details

This object caches the finite element context using [NEML2Assembly.md] and
[NEML2FEInterpolation.md]. The solution is re-interpolated onto the finite
element function space after each explicit solution update.

When an `IntegratedBC` is active, the algebraic element range retains the
locally owned elements adjacent to that boundary so the boundary residual is
assembled without visiting ghost elements. The nodal range is retained when
active `NodalKernel` objects are present. Other domain `Kernel` objects remain
outside the residual loop after the caches are initialized.

## Limitations

- Designed for explicit solves only; no Jacobian contributions are produced by
  the NEML2 kernels in this workflow.
- Requires the NEML2 assembly/interpolation objects to have compatible element
  types within each block (one per element type/order when mixing).
- Regular domain kernels are not evaluated after the NEML2 caches are
  initialized.

## Parameters

!syntax parameters /Executioner/TimeIntegrators/NEML2CentralDifference

## Inputs

!syntax inputs /Executioner/TimeIntegrators/NEML2CentralDifference

!if-end!

!else

!include neml2/neml2_warning.md

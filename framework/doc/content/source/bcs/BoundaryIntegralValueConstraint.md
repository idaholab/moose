# BoundaryIntegralValueConstraint

!syntax description /BCs/BoundaryIntegralValueConstraint

## Description

`BoundaryIntegralValueConstraint` enforces

!equation
\int_{\Gamma} \left(u - \phi_0\right) \, d\Gamma = 0

on a boundary $\Gamma$ using a first-order scalar Lagrange multiplier variable. This makes the
average value of the finite element variable `u` on the specified `boundary` equal to `phi0`.
The variable supplied to [!param](/BCs/BoundaryIntegralValueConstraint/lambda) must use
`family = SCALAR` and `order = FIRST`.

The object assembles both the field-variable equation and the scalar Lagrange multiplier equation,
so no separate `ScalarKernel` is required for the Lagrange multiplier variable. It is the finite
element boundary analogue of [FVBoundaryIntegralValueConstraint.md].

!alert warning
This constraint introduces a saddle-point block with zero diagonal terms. Use a preconditioner that
includes the field and scalar variables. `SMP` with `full = true` is required for the complete
off-diagonal Jacobian block structure, and a suitable shift or factorization is typically needed for
the indefinite matrix.

If `phi0` is supplied as a postprocessor, set its `execute_on` to include `LINEAR` (or `NONLINEAR`)
so the value is current during residual and Jacobian evaluation.

## Example Input Syntax

The Lagrange multiplier variable is declared as a first-order scalar variable:

!listing test/tests/bcs/boundary_integral_value_constraint/boundary_integral_value_constraint.i block=Variables

!listing test/tests/bcs/boundary_integral_value_constraint/boundary_integral_value_constraint.i block=BCs

!syntax parameters /BCs/BoundaryIntegralValueConstraint

!syntax inputs /BCs/BoundaryIntegralValueConstraint

!syntax children /BCs/BoundaryIntegralValueConstraint

# FVTwoVarContinuityConstraint

!syntax description /FVInterfaceKernels/FVTwoVarContinuityConstraint

The constraint is enforced using a Lagrange multiplier variable.

!alert note
Lagrange multipliers do not contribute to the diagonal of the residual, which can hurt
numerical convergence for some linear solvers. For the `lu` (sub-)preconditioner, make sure to use a
`NONZERO` `(sub_)pc_factor_shift_type`.

!alert note
`kernel_coverage_check`, a parameter in the `[Problem]` block, should be set to false, as
the Lagrange multiplier variable is a variable that is not found on any block, only on
the interface.

## Example input file syntax

In this example, we solve two diffusion problems in two adjacent subdomains and force
continuity at the interface, recovering the expected solution of a diffusion problem
spanning the entire domain.

!listing test/tests/fviks/continuity/test.i block=FVInterfaceKernels/interface

!syntax parameters /FVInterfaceKernels/FVTwoVarContinuityConstraint

!syntax inputs /FVInterfaceKernels/FVTwoVarContinuityConstraint

!syntax children /FVInterfaceKernels/FVTwoVarContinuityConstraint

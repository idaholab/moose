# FVScalarLagrangeMultiplierInterface

This class should be inherited to create interface constraints in finite volume.
A Lagrange multiplier, a scalar variable, is used to enforce the constraint on the interface.

!alert note
Lagrange multipliers do not contribute to the diagonal of the residual, which can hurt
numerical convergence for some linear solvers. For the `lu` (sub-)preconditioner, make sure to use a
`NONZERO` `(sub_)pc_factor_shift_type`.

!alert note
`kernel_coverage_check`, a parameter in the `[Problem]` block, should be set to false, as
the Lagrange multiplier variable is a variable that is not found on any block, only on
the interface.

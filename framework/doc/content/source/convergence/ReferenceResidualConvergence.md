# ReferenceResidualConvergence

This [Convergence](Convergence/index.md) is a [/DefaultNonlinearConvergence.md] with a
customized reference residual for its relative convergence checks. See
[ReferenceResidualProblem.md] for more information.

With a variational inequality solver (`-snes_type vinewtonrsls` or `vinewtonssls`, used with
[Bounds](Bounds/index.md)), the residual at degrees of freedom held at a bound (the active set) is
excluded from the residual norms, since the bound constraint holds there in place of the residual
equation.

## Per-variable absolute tolerances

A residual cannot drop below a round-off floor. Variables whose residuals differ by many orders of
magnitude have floors that differ in the same way, so one `nl_abs_tol` can be below the floor of one
variable and far above the floor of another. The
[!param](/Convergence/ReferenceResidualConvergence/variable_nl_abs_tol) parameter replaces
`nl_abs_tol` for the variables that it lists. Variables that it does not list use `nl_abs_tol`. All
variables in a group of [!param](/Convergence/ReferenceResidualConvergence/group_variables) must
have the same tolerance. The `acceptable_multiplier` applies to these tolerances also.

!listing test/tests/convergence/reference_residual_convergence/variable_abs_tol.i block=Convergence

!syntax parameters /Convergence/ReferenceResidualConvergence

!syntax inputs /Convergence/ReferenceResidualConvergence

!syntax children /Convergence/ReferenceResidualConvergence

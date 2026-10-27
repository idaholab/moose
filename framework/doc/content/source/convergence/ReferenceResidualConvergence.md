# ReferenceResidualConvergence

This [Convergence](Convergence/index.md) is a [/DefaultNonlinearConvergence.md] with a
customized reference residual for its relative convergence checks. See
[ReferenceResidualProblem.md] for more information.

With a variational inequality solver (`-snes_type vinewtonrsls` or `vinewtonssls`, used with
[Bounds](Bounds/index.md)), the residual at degrees of freedom held at a bound (the active set) is
excluded from the residual norms, since the bound constraint holds there in place of the residual
equation.

!syntax parameters /Convergence/ReferenceResidualConvergence

!syntax inputs /Convergence/ReferenceResidualConvergence

!syntax children /Convergence/ReferenceResidualConvergence

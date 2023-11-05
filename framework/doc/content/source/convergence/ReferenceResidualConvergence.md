# ReferenceResidualConvergence

The `ReferenceResidualConvergence` object facilitates the assesment of convergence properties of MOOSE applications and allows extensions to provide the user more control over the application behaviour. This convergence object replaces [ReferenceResidualProblem.md], and as such the capabilities are similar to `ReferenceResidualProblem`.

## Description

See [ReferenceResidualProblem.md]. Similar input as the one used in the `[Problem]` block can now be prescribed in the `[Convergence]` block as below

!listing test/tests/convergence/reference_residual/reference_residual.i block=Convergence


!syntax parameters /Convergence/ReferenceResidualConvergence

!syntax inputs /Convergence/ReferenceResidualConvergence

!syntax children /Convergence/ReferenceResidualConvergence

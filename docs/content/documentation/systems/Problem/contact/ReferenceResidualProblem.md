# ReferenceResidualProblem
!syntax description /Problem/ReferenceResidualProblem

## Description

Reference residual is an alternative way to signify convergence
of a timestep.

When using reference residual it is typically acceptable to loosen the relative tolerance for
convergence by an order of magnitude. The difficulty in setting up a `ReferenceResidualProblem`
currently is the requirement of creating an `AuxVariable` for each of the reference residual variables.
Then for each `Kernel` that the corresponding solution variable applies to an additional
line is required to save into the reference residual variable. This requires significant changes to
the input file. If you would like to try using a `ReferenceResidualProblem`, please contact one
of the developers for more detailed instructions of setting it up.

!syntax parameters /Problem/ReferenceResidualProblem

!syntax inputs /Problem/ReferenceResidualProblem

!syntax children /Problem/ReferenceResidualProblem

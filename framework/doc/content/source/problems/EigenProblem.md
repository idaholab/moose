# EigenProblem

!syntax description /Problem/EigenProblem

The `EigenProblem` holds the nonlinear eigen system and organizes the computation
of the residual and Jacobian in the context of an eigen system.

It also takes care of the following customizations of the solve:

- normalization and scaling parameters for eigenvectors
- setting the type of the SLEPc eigenvalue solver
- whether to start solves with free power iterations and how many
- selecting the eigenvector of interest for the residual


!syntax parameters /Problem/EigenProblem

!syntax inputs /Problem/EigenProblem

!syntax children /Problem/EigenProblem

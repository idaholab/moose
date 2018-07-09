# EigenDirichletBC

EigenDirichletBC is provided for Eigenvalue solvers to handle Dirichlet boundary
conditions for the right hand side. It is different from the regular Dirichlet BC.
EigenDirichletBC will always return 0 regardless of the residual, and for the
Jacobian matrix, the corresponding rows are totally zeroed out without adding one
to the matrix diagonal.

!syntax description /BCs/EigenDirichletBC

!syntax parameters /BCs/EigenDirichletBC

!syntax inputs /BCs/EigenDirichletBC

!syntax children /BCs/EigenDirichletBC

!bibtex bibliography
